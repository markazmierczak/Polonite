// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/MemoryStream.h"

#include "Base/Containers/ArrayOps.h"
#include "Base/Io/IoException.h"
#include "Base/Memory/Allocate.h"
#include "Base/Type/Variable.h"

namespace stp {

MemoryStream::MemoryStream() {
}

MemoryStream::~MemoryStream() {
  if (capacity_ > 0) {
    freeMemory(memory_);
  }
}

void MemoryStream::OpenNewBytes() {
  ASSERT(!isOpen());
  open_ = true;
  writable_ = true;
  expandable_ = true;
}

void MemoryStream::AdoptAndOpen(Buffer&& bytes) {
  ASSERT(!isOpen());
  open_ = true;
  writable_ = true;
  expandable_ = true;
  length_ = bytes.size();
  capacity_ = bytes.capacity();
  memory_ = static_cast<byte_t*>(bytes.releaseMemory());
}

void MemoryStream::OpenInternal(void* data, int length, bool writable) {
  ASSERT(!isOpen());
  ASSERT(length >= 0);
  open_ = true;
  writable_ = writable;
  memory_ = static_cast<byte_t*>(data);
  length_ = length;
}

Buffer MemoryStream::CloseAndrelease() {
  ASSERT(expandable_);
  byte_t* ptr = exchange(memory_, nullptr);
  int size = exchange(length_, 0);
  int capacity = exchange(capacity_, 0);
  return Buffer::adoptMemory(ptr, size, capacity);
}

void MemoryStream::close() {
  ASSERT(isOpen());

  if (capacity_ > 0) {
    freeMemory(memory_);
  }
  memory_ = nullptr;
  length_ = 0;
  capacity_ = 0;
  position_ = 0;

  open_ = false;
  writable_ = false;
  expandable_ = false;
}

bool MemoryStream::isOpen() const noexcept {
  return open_;
}

int MemoryStream::readAtMost(MutableBufferSpan output) {
  ASSERT(canRead());
  int available = length_ - position_;
  int n = output.size();

  if (available < n) {
    if (available <= 0)
      return 0;
    n = available;
  }
  if (n) {
    auto* output_bytes = static_cast<byte_t*>(output.data());
    uninitializedCopy(output_bytes, memory_ + position_, n);
    position_ += n;
  }
  return n;
}

void MemoryStream::write(BufferSpan input) {
  positionalWrite(position_, input);
  position_ += input.size();
}

void MemoryStream::positionalRead(int64_t offset, MutableBufferSpan output) {
  ASSERT(canRead());
  ASSERT(offset >= 0);

  int n = output.size();
  if (length_ - offset < n)
    throw EndOfStreamException();
  if (n) {
    auto* output_bytes = static_cast<byte_t*>(output.data());
    uninitializedCopy(output_bytes, memory_ + offset, n);
  }
}

void MemoryStream::positionalWrite(int64_t offset, BufferSpan input) {
  ASSERT(canWrite());
  ASSERT(offset >= 0);

  int n = input.size();
  if (offset > MaxCapacity_ - n)
    throw Exception::with(IoException(), "attempted to write past memory limit");

  int after_pos = static_cast<int>(offset) + n;
  if (after_pos > length_) {
    ensureCapacity(after_pos);
    length_ = after_pos;
  }
  if (n) {
    auto* input_bytes = static_cast<const byte_t*>(input.data());
    uninitializedCopy(memory_ + offset, input_bytes, n);
  }
}

void MemoryStream::writeByte(byte_t byte) {
  ASSERT(canWrite());

  if (position_ >= length_) {
    int new_length = position_ + 1;
    if (new_length > capacity_) {
      if (new_length > MaxCapacity_)
        throw Exception::with(IoException(), "attempted to write past memory limit");
      ensureCapacity(new_length);
    }
    if (position_ > length_) {
      // Clear the garbage in memory.
      uninitializedInit(memory_, position_ - length_);
    }
    length_ = new_length;
  }
  memory_[position_++] = byte;
}

int MemoryStream::tryReadByte() {
  ASSERT(canRead());
  if (position_ >= length_)
    return -1;
  return memory_[position_++];
}

int64_t MemoryStream::seek(int64_t offset, SeekOrigin origin) {
  ASSERT(canSeek());

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
    throw Exception::with(IoException(), "attempted to seek before the beginning of stream");

  position_ = new_pos;
  return new_pos;
}

void MemoryStream::flush() {
  ASSERT(isOpen());
  // Nothing to do, we directly write to memory.
}

bool MemoryStream::canRead() {
  return isOpen();
}

bool MemoryStream::canWrite() {
  return writable_;
}

bool MemoryStream::canSeek() {
  return isOpen();
}

void MemoryStream::setLength(int64_t new_length) {
  ASSERT(new_length >= 0);

  if (new_length > length_) {
    if (new_length >= MaxCapacity_)
      throw Exception::with(IoException(), "attempted to resize past memory limit");

    ensureCapacity(static_cast<int>(new_length));
    // Zero the memory acquired by resizing up.
    uninitializedInit(memory_ + length_, new_length - length_);
  } else {
    // See if new length make memory small enough to lower capacity.
    int new_capacity = static_cast<int>(new_length);
    if (new_capacity < MinCapacity_)
      new_capacity = MinCapacity_;

    if (new_capacity <= (capacity_ >> 1)) {
      memory_ = (byte_t*)reallocateMemory(memory_, new_capacity);
      ASSERT(memory_ != nullptr, "contracting memory failed");
      capacity_ = new_capacity;
    }
  }
  length_ = new_length;
}

int64_t MemoryStream::getLength() {
  return length_;
}

void MemoryStream::setPosition(int64_t new_position) {
  ASSERT(canSeek());
  ASSERT(new_position >= 0);
  if (new_position >= MaxCapacity_)
    throw Exception::with(IoException(), "cannot seek past memory limit");
  position_ = static_cast<int>(new_position);
}

int64_t MemoryStream::getPosition() {
  return position_;
}

void MemoryStream::ensureCapacity(int request) {
  ASSERT(0 <= request && request <= MaxCapacity_);

  if (request <= capacity_)
    return; // already satisfied

  if (!expandable_)
    throw Exception::with(IoException(), "unable to resize the non-expandable stream");

  int doubled_capacity = capacity_ << 1;
  if (doubled_capacity > MaxCapacity_)
    request = MaxCapacity_;
  else if (request < doubled_capacity)
    request = doubled_capacity;
  else if (request < MinCapacity_)
    request = MinCapacity_;

  memory_ = (byte_t*)reallocateMemory(memory_, request);
  capacity_ = request;
}

} // namespace stp
