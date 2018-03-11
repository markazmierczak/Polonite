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
  DISALLOW_COPY_AND_ASSIGN(Stream);
 public:
  static Stream& nullStream();

  Stream() = default;
  virtual ~Stream() {}

  virtual void close() = 0;

  virtual bool isOpen() const = 0;

  virtual int readAtMost(MutableBufferSpan output) = 0;
  void read(MutableBufferSpan output);

  virtual void write(BufferSpan input) = 0;

  virtual void positionalRead(int64_t offset, MutableBufferSpan output);
  virtual void positionalWrite(int64_t offset, BufferSpan input);

  virtual void writeByte(byte_t byte);
  virtual int tryReadByte();
  byte_t readByte();

  virtual int64_t seek(int64_t offset, SeekOrigin origin) = 0;

  virtual void flush() = 0;

  virtual bool canRead() = 0;
  virtual bool canWrite() = 0;
  virtual bool canSeek() = 0;

  // The file position is undefined after this operation.
  virtual void setLength(int64_t length) = 0;
  virtual int64_t getLength() = 0;

  virtual void setPosition(int64_t position) = 0;
  virtual int64_t getPosition() = 0;
};

} // namespace stp

#endif // STP_BASE_IO_STREAM_H_
