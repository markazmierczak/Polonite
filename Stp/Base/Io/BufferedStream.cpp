// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/BufferedStream.h"

#include "Base/Containers/ArrayOps.h"
#include "Base/Memory/Allocate.h"

namespace stp {

BufferedStream::BufferedStream() {}

BufferedStream::~BufferedStream() {
  if (owned_ && underlying_)
    delete underlying_;

  // Free internal buffer if any.
  if (buffer_)
    freeMemory(buffer_);
}

void BufferedStream::EnsureBufferAllocated() {
  if (!buffer_ && buffer_size_ > 0)
    buffer_ = (byte_t*)allocateMemory(buffer_size_);
}

void BufferedStream::SetBufferSize(int new_size) {
  ASSERT(new_size > 0);
  if (buffer_) {
    FlushBuffers();
    buffer_ = (byte_t*)reallocateMemory(buffer_, new_size);
  }
  buffer_size_ = new_size;
}

void BufferedStream::FlushWriteBuffer() {
  ASSERT(HasPendingWrite());
  underlying_->Write(BufferSpan(buffer_, write_pos_));
  write_pos_ = 0;
}

void BufferedStream::FlushReadBuffer() {
  ASSERT(HasPendingRead());

  int offset = read_pos_ - read_len_;

  read_pos_ = 0;
  read_len_ = 0;

  if (offset != 0)
    underlying_->Seek(offset, SeekOrigin::Current);
}

void BufferedStream::FlushBuffers() {
  if (HasPendingWrite()) {
    FlushWriteBuffer();
  } else if (HasPendingRead()) {
    // Ignore the flush if stream is not seekable.
    if (underlying_->CanSeek())
      FlushReadBuffer();
  }
}

void BufferedStream::ClearReadBufferBeforeWrite() {
  if (read_pos_ == read_len_) {
    read_pos_ = 0;
    read_len_ = 0;
  } else {
    ASSERT(underlying_->CanSeek(), "underlying stream must be seekable for mixed R/W operations");
    FlushReadBuffer();
  }
}

int BufferedStream::ReadFromBuffer(MutableBufferSpan output) {
  int in_buffer = read_len_ - read_pos_;
  if (in_buffer == 0)
    return 0;

  int count = output.size();
  if (count > in_buffer)
    count = in_buffer;

  auto* output_bytes = static_cast<byte_t*>(output.data());
  uninitializedCopy(output_bytes, buffer_ + read_pos_, count);
  read_pos_ += count;
  return count;
}

int BufferedStream::WriteToBuffer(BufferSpan input) {
  ASSERT(write_pos_ >= 0);

  int bytes_to_write = min(buffer_size_ - write_pos_, input.size());
  if (bytes_to_write == 0)
    return 0;

  EnsureBufferAllocated();

  auto* input_bytes = static_cast<const byte_t*>(input.data());
  uninitializedCopy(buffer_ + write_pos_, input_bytes, bytes_to_write);
  write_pos_ += bytes_to_write;
  return bytes_to_write;
}

bool BufferedStream::IsOpen() const noexcept {
  return underlying_ != nullptr;
}

void BufferedStream::OpenInternal(Stream* underlying, bool owned = false) {
  ASSERT(underlying->IsOpen(), "given underlying stream must be open");
  ASSERT(!IsOpen());
  underlying_ = underlying;
  owned_ = owned;
}

void BufferedStream::Close() {
  ASSERT(IsOpen());

  if (HasPendingWrite()) {
    FlushWriteBuffer();
  } else {
    read_pos_ = 0;
    read_len_ = 0;
  }

  Stream* underlying = exchange(underlying_, nullptr);
  if (owned_) {
    // Call Close directly instead of indirectly through destructor.
    // This enables exception chaining.
    OwnPtr<Stream> guard(underlying);
    underlying->Close();
  }
}

void BufferedStream::Flush() {
  ASSERT(IsOpen());
  FlushBuffers();
  underlying_->Flush();
}

int BufferedStream::ReadAtMost(MutableBufferSpan output) {
  ASSERT(CanRead());

  // Try to satisfy the request with data from internal buffer.
  int from_buffer = ReadFromBuffer(output);
  if (from_buffer == output.size())
    return output.size();

  if (from_buffer > 0)
    output.removePrefix(from_buffer);

  int already_satisfied = from_buffer;

  if (HasPendingWrite()) {
    FlushWriteBuffer();
  } else {
    read_pos_ = 0;
    read_len_ = 0;
  }
  // Bypass the buffer if caller requests more bytes than buffer can hold.
  if (output.size() >= buffer_size_) {
    int direct_read = underlying_->ReadAtMost(output);
    return direct_read + already_satisfied;
  }

  EnsureBufferAllocated();
  read_len_ = underlying_->ReadAtMost(MutableBufferSpan(buffer_, buffer_size_));

  from_buffer = ReadFromBuffer(output);
  return from_buffer + already_satisfied;
}

void BufferedStream::Write(BufferSpan input) {
  ASSERT(CanWrite());

  if (write_pos_ == 0)
    ClearReadBufferBeforeWrite();

  // This heuristic decides whether to use or not to use the buffer.
  int total_count = write_pos_ + input.size();
  bool use_buffer = total_count <= buffer_size_ * 2;

  if (use_buffer) {
    int wrote = WriteToBuffer(input);
    if (write_pos_ == buffer_size_) {
      // The source spans two buffers. Make second write.
      input.removePrefix(wrote);
      FlushWriteBuffer();
      wrote = WriteToBuffer(input);
    }
    ASSERT(wrote == input.size());
  } else {
    FlushWriteBuffer();
    underlying_->Write(input);
  }
}

void BufferedStream::WriteByte(byte_t byte) {
  if (write_pos_ == 0) {
    ASSERT(CanWrite());
    ClearReadBufferBeforeWrite();
    EnsureBufferAllocated();
  } else if (write_pos_ == buffer_size_) {
    FlushWriteBuffer();
  }
  buffer_[write_pos_++] = byte;
}

int BufferedStream::TryReadByte() {
  if (HasPendingRead())
    return buffer_[read_pos_++];

  ASSERT(CanRead());
  if (HasPendingWrite())
    FlushWriteBuffer();

  EnsureBufferAllocated();
  read_len_ = underlying_->ReadAtMost(MutableBufferSpan(buffer_, buffer_size_));
  read_pos_ = 0;

  if (read_len_ == 0)
    return -1;

  return buffer_[read_pos_++];
}

bool BufferedStream::CanRead() {
  return underlying_ && underlying_->CanRead();
}

bool BufferedStream::CanWrite() {
  return underlying_ && underlying_->CanWrite();
}

bool BufferedStream::CanSeek() {
  return underlying_ && underlying_->CanSeek();
}

void BufferedStream::SetLength(int64_t length) {
  ASSERT(IsOpen());
  FlushBuffers();
  underlying_->SetLength(length);
}

int64_t BufferedStream::GetLength() {
  if (HasPendingWrite())
    FlushWriteBuffer();
  return underlying_->GetLength();
}

void BufferedStream::SetPosition(int64_t position) {
  ASSERT(position >= 0);
  ASSERT(CanSeek());

  if (HasPendingWrite())
    FlushWriteBuffer();

  read_pos_ = 0;
  read_len_ = 0;
  underlying_->SetPosition(position);
}

int64_t BufferedStream::GetPosition() {
  ASSERT(CanSeek());
  int64_t underlying_pos = underlying_->GetPosition();
  return underlying_pos + (read_pos_ - read_len_ + write_pos_);
}

int64_t BufferedStream::Seek(int64_t offset, SeekOrigin origin) {
  ASSERT(CanSeek());

  if (HasPendingWrite()) {
    FlushWriteBuffer();
    return underlying_->Seek(offset, origin);
  }

  int64_t old_pos = -1;
  if (read_len_ > 0) {
    if (origin == SeekOrigin::Current)
      offset -= read_len_ - read_pos_;
    old_pos = GetPosition();
  }

  int64_t new_pos = underlying_->Seek(offset, origin);

  if (read_len_ > 0) {
    int64_t new_read_pos = new_pos - old_pos + read_pos_;
    if (0 <= new_read_pos && new_read_pos < read_len_) {
      read_pos_ = static_cast<int>(new_read_pos);
      underlying_->Seek(read_len_ - read_pos_, SeekOrigin::Current);
    } else {
      read_pos_ = 0;
      read_len_ = 0;
    }
  }
  ASSERT(new_pos == GetPosition());
  return new_pos;
}

} // namespace stp
