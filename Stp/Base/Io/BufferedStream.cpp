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

void BufferedStream::ensureBufferAllocated() {
  if (!buffer_ && buffer_size_ > 0)
    buffer_ = (byte_t*)allocateMemory(buffer_size_);
}

void BufferedStream::setBufferSize(int new_size) {
  ASSERT(new_size > 0);
  if (buffer_) {
    flushBuffers();
    buffer_ = (byte_t*)reallocateMemory(buffer_, new_size);
  }
  buffer_size_ = new_size;
}

void BufferedStream::flushWriteBuffer() {
  ASSERT(hasPendingWrite());
  underlying_->write(BufferSpan(buffer_, write_pos_));
  write_pos_ = 0;
}

void BufferedStream::flushReadBuffer() {
  ASSERT(hasPendingRead());

  int offset = read_pos_ - read_len_;

  read_pos_ = 0;
  read_len_ = 0;

  if (offset != 0)
    underlying_->seek(offset, SeekOrigin::Current);
}

void BufferedStream::flushBuffers() {
  if (hasPendingWrite()) {
    flushWriteBuffer();
  } else if (hasPendingRead()) {
    // Ignore the flush if stream is not seekable.
    if (underlying_->canSeek())
      flushReadBuffer();
  }
}

void BufferedStream::clearReadBufferBeforeWrite() {
  if (read_pos_ == read_len_) {
    read_pos_ = 0;
    read_len_ = 0;
  } else {
    ASSERT(underlying_->canSeek(), "underlying stream must be seekable for mixed R/W operations");
    flushReadBuffer();
  }
}

int BufferedStream::readFromBuffer(MutableBufferSpan output) {
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

int BufferedStream::writeToBuffer(BufferSpan input) {
  ASSERT(write_pos_ >= 0);

  int bytes_to_write = min(buffer_size_ - write_pos_, input.size());
  if (bytes_to_write == 0)
    return 0;

  ensureBufferAllocated();

  auto* input_bytes = static_cast<const byte_t*>(input.data());
  uninitializedCopy(buffer_ + write_pos_, input_bytes, bytes_to_write);
  write_pos_ += bytes_to_write;
  return bytes_to_write;
}

bool BufferedStream::isOpen() const noexcept {
  return underlying_ != nullptr;
}

void BufferedStream::openInternal(Stream* underlying, bool owned = false) {
  ASSERT(underlying->isOpen(), "given underlying stream must be open");
  ASSERT(!isOpen());
  underlying_ = underlying;
  owned_ = owned;
}

void BufferedStream::close() {
  ASSERT(isOpen());

  if (hasPendingWrite()) {
    flushWriteBuffer();
  } else {
    read_pos_ = 0;
    read_len_ = 0;
  }

  Stream* underlying = exchange(underlying_, nullptr);
  if (owned_) {
    // Call Close directly instead of indirectly through destructor.
    // This enables exception chaining.
    OwnPtr<Stream> guard(underlying);
    underlying->close();
  }
}

void BufferedStream::flush() {
  ASSERT(isOpen());
  flushBuffers();
  underlying_->flush();
}

int BufferedStream::readAtMost(MutableBufferSpan output) {
  ASSERT(canRead());

  // Try to satisfy the request with data from internal buffer.
  int from_buffer = readFromBuffer(output);
  if (from_buffer == output.size())
    return output.size();

  if (from_buffer > 0)
    output.removePrefix(from_buffer);

  int already_satisfied = from_buffer;

  if (hasPendingWrite()) {
    flushWriteBuffer();
  } else {
    read_pos_ = 0;
    read_len_ = 0;
  }
  // Bypass the buffer if caller requests more bytes than buffer can hold.
  if (output.size() >= buffer_size_) {
    int direct_read = underlying_->readAtMost(output);
    return direct_read + already_satisfied;
  }

  ensureBufferAllocated();
  read_len_ = underlying_->readAtMost(MutableBufferSpan(buffer_, buffer_size_));

  from_buffer = readFromBuffer(output);
  return from_buffer + already_satisfied;
}

void BufferedStream::write(BufferSpan input) {
  ASSERT(canWrite());

  if (write_pos_ == 0)
    clearReadBufferBeforeWrite();

  // This heuristic decides whether to use or not to use the buffer.
  int total_count = write_pos_ + input.size();
  bool use_buffer = total_count <= buffer_size_ * 2;

  if (use_buffer) {
    int wrote = writeToBuffer(input);
    if (write_pos_ == buffer_size_) {
      // The source spans two buffers. Make second write.
      input.removePrefix(wrote);
      flushWriteBuffer();
      wrote = writeToBuffer(input);
    }
    ASSERT(wrote == input.size());
  } else {
    flushWriteBuffer();
    underlying_->write(input);
  }
}

void BufferedStream::writeByte(byte_t byte) {
  if (write_pos_ == 0) {
    ASSERT(canWrite());
    clearReadBufferBeforeWrite();
    ensureBufferAllocated();
  } else if (write_pos_ == buffer_size_) {
    flushWriteBuffer();
  }
  buffer_[write_pos_++] = byte;
}

int BufferedStream::tryReadByte() {
  if (hasPendingRead())
    return buffer_[read_pos_++];

  ASSERT(canRead());
  if (hasPendingWrite())
    flushWriteBuffer();

  ensureBufferAllocated();
  read_len_ = underlying_->readAtMost(MutableBufferSpan(buffer_, buffer_size_));
  read_pos_ = 0;

  if (read_len_ == 0)
    return -1;

  return buffer_[read_pos_++];
}

bool BufferedStream::canRead() {
  return underlying_ && underlying_->canRead();
}

bool BufferedStream::canWrite() {
  return underlying_ && underlying_->canWrite();
}

bool BufferedStream::canSeek() {
  return underlying_ && underlying_->canSeek();
}

void BufferedStream::setLength(int64_t length) {
  ASSERT(isOpen());
  flushBuffers();
  underlying_->setLength(length);
}

int64_t BufferedStream::getLength() {
  if (hasPendingWrite())
    flushWriteBuffer();
  return underlying_->getLength();
}

void BufferedStream::setPosition(int64_t position) {
  ASSERT(position >= 0);
  ASSERT(canSeek());

  if (hasPendingWrite())
    flushWriteBuffer();

  read_pos_ = 0;
  read_len_ = 0;
  underlying_->setPosition(position);
}

int64_t BufferedStream::getPosition() {
  ASSERT(canSeek());
  int64_t underlying_pos = underlying_->getPosition();
  return underlying_pos + (read_pos_ - read_len_ + write_pos_);
}

int64_t BufferedStream::seek(int64_t offset, SeekOrigin origin) {
  ASSERT(canSeek());

  if (hasPendingWrite()) {
    flushWriteBuffer();
    return underlying_->seek(offset, origin);
  }

  int64_t old_pos = -1;
  if (read_len_ > 0) {
    if (origin == SeekOrigin::Current)
      offset -= read_len_ - read_pos_;
    old_pos = getPosition();
  }

  int64_t new_pos = underlying_->seek(offset, origin);

  if (read_len_ > 0) {
    int64_t new_read_pos = new_pos - old_pos + read_pos_;
    if (0 <= new_read_pos && new_read_pos < read_len_) {
      read_pos_ = static_cast<int>(new_read_pos);
      underlying_->seek(read_len_ - read_pos_, SeekOrigin::Current);
    } else {
      read_pos_ = 0;
      read_len_ = 0;
    }
  }
  ASSERT(new_pos == getPosition());
  return new_pos;
}

} // namespace stp
