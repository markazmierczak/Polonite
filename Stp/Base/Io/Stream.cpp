// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/Stream.h"

#include "Base/Containers/ArrayOps.h"
#include "Base/Debug/Assert.h"
#include "Base/Io/IoException.h"

namespace stp {

void Stream::writeByte(byte_t b) {
  write(BufferSpan(&b, 1));
}

int Stream::tryReadByte() {
  byte_t b;
  int rv = readAtMost(MutableBufferSpan(&b, 1));
  return rv == 1 ? static_cast<int>(b) : -1;
}

byte_t Stream::readByte() {
  int b = tryReadByte();
  if (b >= 0) {
    ASSERT(b < 256);
    return static_cast<byte_t>(b);
  }
  throw EndOfStreamException();
}

void Stream::positionalRead(int64_t offset, MutableBufferSpan output) {
  setPosition(offset);
  read(output);
}

void Stream::positionalWrite(int64_t offset, BufferSpan input) {
  setPosition(offset);
  write(input);
}

void Stream::read(MutableBufferSpan buffer) {
  int rv = readAtMost(buffer);
  if (rv != buffer.size())
    throw EndOfStreamException();
}

namespace {

class NullStream final : public Stream {
 public:
  void close() override { ASSERT(false); }
  bool isOpen() const override { return true; }
  int readAtMost(MutableBufferSpan output) override { return 0; }
  void write(BufferSpan output) override {}
  void positionalRead(int64_t offset, MutableBufferSpan output) override;
  void positionalWrite(int64_t offset, BufferSpan input) override {}
  void writeByte(byte_t byte) override {}
  int tryReadByte() override { return -1; }
  int64_t seek(int64_t offset, SeekOrigin origin) override { return 0; }
  void flush() override {}
  bool canRead() override { return true; }
  bool canWrite() override { return true; }
  bool canSeek() override { return true; }
  void setLength(int64_t length) override {}
  int64_t getLength() override { return 0; }
  void setPosition(int64_t position) override {}
  int64_t getPosition() override { return 0; }
};

void NullStream::positionalRead(int64_t offset, MutableBufferSpan output) {
  output.fill(0);
}

} // namespace

Stream& Stream::nullStream() {
  static AlignedStorage<NullStream> g_storage;
  return *new(g_storage.bytes) NullStream();
}

} // namespace stp
