// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_BUFFEREDSTREAM_H_
#define STP_BASE_IO_BUFFEREDSTREAM_H_

#include "Base/Io/Stream.h"
#include "Base/Memory/OwnPtr.h"

namespace stp {

// NOTE: For mixed read/write operation the underlying stream is required to
//       be seekable.
class BASE_EXPORT BufferedStream final : public Stream {
  DISALLOW_COPY_AND_ASSIGN(BufferedStream);
 public:
  static constexpr int DefaultBufferSize = 4096;

  BufferedStream();
  ~BufferedStream() override;

  ALWAYS_INLINE Stream* getUnderlying() const { return underlying_; }

  // The |new_size| must be greater than zero.
  // The size of buffer can be changed only when buffers are flushed.
  // Otherwise the behavior is undefined.
  void setBufferSize(int new_size);
  ALWAYS_INLINE int getBufferSize() const { return buffer_size_; }

  void flushBuffers();

  void open(OwnPtr<Stream> underlying) { openInternal(underlying.release(), true); }
  void open(Stream* underlying) { openInternal(underlying, false); }

  void close() override;
  bool isOpen() const noexcept override;
  int readAtMost(MutableBufferSpan output) override;
  void write(BufferSpan input) override;
  void writeByte(byte_t byte) override;
  int tryReadByte() override;
  int64_t seek(int64_t offset, SeekOrigin origin) override;
  void flush() override;
  bool canRead() override;
  bool canWrite() override;
  bool canSeek() override;
  void setLength(int64_t length) override;
  int64_t getLength() override;
  void setPosition(int64_t position) override;
  int64_t getPosition() override;

 private:
  Stream* underlying_ = nullptr;
  // A buffer for reading/writing. Allocated on first use.
  byte_t* buffer_ = nullptr;
  // Write pointer within |buffer_|.
  int write_pos_ = 0;
  // Read pointer and length to enable ring buffer for reading.
  // |read_len_| can be negative when error occurred in read operation.
  int read_pos_ = 0;
  int read_len_ = 0;
  // Number of bytes in the |buffer_|.
  int buffer_size_ = DefaultBufferSize;
  // Whether underlying is owned.
  bool owned_ = false;

  void openInternal(Stream* underlying, bool owned);

  bool hasPendingWrite() const { return write_pos_ > 0; }
  bool hasPendingRead() const { return read_pos_ < read_len_; }

  void flushWriteBuffer();
  void flushReadBuffer();

  void clearReadBufferBeforeWrite();

  int readFromBuffer(MutableBufferSpan output);
  int writeToBuffer(BufferSpan input);

  void ensureBufferAllocated();
};

} // namespace stp

#endif // STP_BASE_IO_BUFFEREDSTREAM_H_
