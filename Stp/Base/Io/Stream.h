// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_STREAM_H_
#define STP_BASE_IO_STREAM_H_

#include "Base/Containers/BufferSpan.h"

namespace stp {

// This explicit mapping matches both FILE_ on Windows and SEEK_ on Linux.
enum class SeekOrigin {
  Begin = 0,
  Current = 1,
  End = 2,
};

class BASE_EXPORT Stream {
 public:
  static Stream& Null();

  Stream() = default;
  virtual ~Stream() {}

  virtual void Close() = 0;

  virtual bool IsOpen() const noexcept = 0;

  virtual int ReadAtMost(MutableBufferSpan output) = 0;
  void Read(MutableBufferSpan output);

  virtual void Write(BufferSpan input) = 0;

  virtual void PositionalRead(int64_t offset, MutableBufferSpan output);
  virtual void PositionalWrite(int64_t offset, BufferSpan input);

  virtual void WriteByte(byte_t byte);
  virtual int TryReadByte();
  byte_t ReadByte();

  virtual int64_t Seek(int64_t offset, SeekOrigin origin) = 0;

  virtual void Flush() = 0;

  virtual bool CanRead() = 0;
  virtual bool CanWrite() = 0;
  virtual bool CanSeek() = 0;

  // The file position is undefined after this operation.
  virtual void SetLength(int64_t length) = 0;
  virtual int64_t GetLength() = 0;

  virtual void SetPosition(int64_t position) = 0;
  virtual int64_t GetPosition() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(Stream);
};

} // namespace stp

#endif // STP_BASE_IO_STREAM_H_
