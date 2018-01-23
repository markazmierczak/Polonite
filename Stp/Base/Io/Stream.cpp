// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/Stream.h"

#include "Base/Containers/ArrayOps.h"
#include "Base/Debug/Assert.h"
#include "Base/Io/IoException.h"

namespace stp {

void Stream::WriteByte(byte_t b) {
  Write(BufferSpan(&b, 1));
}

int Stream::TryReadByte() {
  byte_t b;
  int rv = ReadAtMost(MutableBufferSpan(&b, 1));
  return rv == 1 ? static_cast<int>(b) : -1;
}

byte_t Stream::ReadByte() {
  int b = TryReadByte();
  if (b >= 0) {
    ASSERT(b < 256);
    return static_cast<byte_t>(b);
  }
  throw EndOfStreamException();
}

void Stream::PositionalRead(int64_t offset, MutableBufferSpan output) {
  SetPosition(offset);
  Read(output);
}

void Stream::PositionalWrite(int64_t offset, BufferSpan input) {
  SetPosition(offset);
  Write(input);
}

void Stream::Read(MutableBufferSpan buffer) {
  int rv = ReadAtMost(buffer);
  if (rv != buffer.size())
    throw EndOfStreamException();
}

namespace {

class NullStream final : public Stream {
 public:
  void Close() override { ASSERT(false); }
  bool IsOpen() const noexcept override { return true; }
  int ReadAtMost(MutableBufferSpan output) override { return 0; }
  void Write(BufferSpan output) override {}
  void PositionalRead(int64_t offset, MutableBufferSpan output) override;
  void PositionalWrite(int64_t offset, BufferSpan input) override {}
  void WriteByte(byte_t byte) override {}
  int TryReadByte() override { return -1; }
  int64_t Seek(int64_t offset, SeekOrigin origin) override { return 0; }
  void Flush() override {}
  bool CanRead() override { return true; }
  bool CanWrite() override { return true; }
  bool CanSeek() override { return true; }
  void SetLength(int64_t length) override {}
  int64_t GetLength() override { return 0; }
  void SetPosition(int64_t position) override {}
  int64_t GetPosition() override { return 0; }
};

void NullStream::PositionalRead(int64_t offset, MutableBufferSpan output) {
  Fill(output, 0);
}

} // namespace

Stream& Stream::Null() {
  static AlignedStorage<NullStream> g_storage;
  return *new(g_storage.bytes) NullStream();
}

} // namespace stp
