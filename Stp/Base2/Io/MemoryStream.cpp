// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/MemoryStream.h"

#include "Base/Containers/ArrayOps.h"
#include "Base/Io/IoException.h"
#include "Base/Mem/Allocate.h"
#include "Base/Type/Variable.h"

namespace stp {

MemoryStream::MemoryStream() {
}

MemoryStream::~MemoryStream() {
  if (capacity_ > 0)
    Free(memory_);
}

void MemoryStream::OpenNewBytes() {
  ASSERT(!IsOpen());
  open_ = true;
  writable_ = true;
  expandable_ = true;
}

void MemoryStream::AdoptAndOpen(Buffer&& bytes) {
  ASSERT(!IsOpen());
  open_ = true;
  writable_ = true;
  expandable_ = true;
  length_ = bytes.size();
  capacity_ = bytes.capacity();
  memory_ = static_cast<byte_t*>(bytes.ReleaseMemory());
}

void MemoryStream::OpenInternal(void* data, int length, bool writable) {
  ASSERT(!IsOpen());
  ASSERT(length >= 0);
  open_ = true;
  writable_ = writable;
  memory_ = static_cast<byte_t*>(data);
  length_ = length;
}

Buffer MemoryStream::CloseAndRelease() {
  ASSERT(expandable_);
  byte_t* ptr = Exchange(memory_, nullptr);
  int size = Exchange(length_, 0);
  int capacity = Exchange(capacity_, 0);
  return Buffer::AdoptMemory(ptr, size, capacity);
}

void MemoryStream::Close() {
  ASSERT(IsOpen());

  if (capacity_ > 0)
    Free(memory_);

  memory_ = nullptr;
  length_ = 0;
  capacity_ = 0;
  position_ = 0;

  open_ = false;
  writable_ = false;
  expandable_ = false;
}

bool MemoryStream::IsOpen() const noexcept {
  return open_;
}

int MemoryStream::ReadAtMost(MutableBufferSpan output) {
  ASSERT(CanRead());
  int available = length_ - position_;
  int n = output.size();

  if (available < n) {
    if (available <= 0)
      return 0;
    n = available;
  }
  if (n) {
    auto* output_bytes = static_cast<byte_t*>(output.data());
    UninitializedCopy(output_bytes, memory_ + position_, n);
    position_ += n;
  }
  return n;
}

void MemoryStream::Write(BufferSpan input) {
  PositionalWrite(position_, input);
  position_ += input.size();
}

void MemoryStream::PositionalRead(int64_t offset, MutableBufferSpan output) {
  ASSERT(CanRead());
  ASSERT(offset >= 0);

  int n = output.size();
  if (length_ - offset < n)
    throw EndOfStreamException();
  if (n) {
    auto* output_bytes = static_cast<byte_t*>(output.data());
    UninitializedCopy(output_bytes, memory_ + offset, n);
  }
}

void MemoryStream::PositionalWrite(int64_t offset, BufferSpan input) {
  ASSERT(CanWrite());
  ASSERT(offset >= 0);

  int n = input.size();
  if (offset > MaxCapacity_ - n)
    throw Exception::With(IoException(), "attempted to write past memory limit");

  int after_pos = static_cast<int>(offset) + n;
  if (after_pos > length_) {
    EnsureCapacity(after_pos);
    length_ = after_pos;
  }
  if (n) {
    auto* input_bytes = static_cast<const byte_t*>(input.data());
    UninitializedCopy(memory_ + offset, input_bytes, n);
  }
}

void MemoryStream::WriteByte(byte_t byte) {
  ASSERT(CanWrite());

  if (position_ >= length_) {
    int new_length = position_ + 1;
    if (new_length > capacity_) {
      if (new_length > MaxCapacity_)
        throw Exception::With(IoException(), "attempted to write past memory limit");
      EnsureCapacity(new_length);
    }
    if (position_ > length_) {
      // Clear the garbage in memory.
      UninitializedInit(memory_, position_ - length_);
    }
    length_ = new_length;
  }
  memory_[position_++] = byte;
}

int MemoryStream::TryReadByte() {
  ASSERT(CanRead());
  if (position_ >= length_)
    return -1;
  return memory_[position_++];
}

int64_t MemoryStream::Seek(int64_t offset, SeekOrigin origin) {
  ASSERT(CanSeek());

  int64_t new_pos;
  switch (origin) {
    case SeekOrigin::Begin:
      new_pos = offset;
      break;
    case SeekOrigin::Current:
      new_pos = position_ + offset;
      break;
    case SeekOrigin::End:
      new_pos = length_ + offset;
      break;
    default:
      UNREACHABLE(new_pos = -1);
  }
  if (new_pos < 0)
    throw Exception::With(IoException(), "attempted to seek before the beginning of stream");

  position_ = new_pos;
  return new_pos;
}

void MemoryStream::Flush() {
  ASSERT(IsOpen());
  // Nothing to do, we directly write to memory.
}

bool MemoryStream::CanRead() {
  return IsOpen();
}

bool MemoryStream::CanWrite() {
  return writable_;
}

bool MemoryStream::CanSeek() {
  return IsOpen();
}

void MemoryStream::SetLength(int64_t new_length) {
  ASSERT(new_length >= 0);

  if (new_length > length_) {
    if (new_length >= MaxCapacity_)
      throw Exception::With(IoException(), "attempted to resize past memory limit");

    EnsureCapacity(static_cast<int>(new_length));
    // Zero the memory acquired by resizing up.
    UninitializedInit(memory_ + length_, new_length - length_);
  } else {
    // See if new length make memory small enough to lower capacity.
    int new_capacity = static_cast<int>(new_length);
    if (new_capacity < MinCapacity_)
      new_capacity = MinCapacity_;

    if (new_capacity <= (capacity_ >> 1)) {
      memory_ = Reallocate(memory_, ToUnsigned(new_capacity));
      ASSERT(memory_ != nullptr, "contracting memory failed");
      capacity_ = new_capacity;
    }
  }
  length_ = new_length;
}

int64_t MemoryStream::GetLength() {
  return length_;
}

void MemoryStream::SetPosition(int64_t new_position) {
  ASSERT(CanSeek());
  ASSERT(new_position >= 0);
  if (new_position >= MaxCapacity_)
    throw Exception::With(IoException(), "cannot seek past memory limit");
  position_ = static_cast<int>(new_position);
}

int64_t MemoryStream::GetPosition() {
  return position_;
}

void MemoryStream::EnsureCapacity(int request) {
  ASSERT(0 <= request && request <= MaxCapacity_);

  if (request <= capacity_)
    return; // already satisfied

  if (!expandable_)
    throw Exception::With(IoException(), "unable to resize the non-expandable stream");

  int doubled_capacity = capacity_ << 1;
  if (doubled_capacity > MaxCapacity_)
    request = MaxCapacity_;
  else if (request < doubled_capacity)
    request = doubled_capacity;
  else if (request < MinCapacity_)
    request = MinCapacity_;

  memory_ = Reallocate(memory_, ToUnsigned(request));
  capacity_ = request;
}

} // namespace stp
