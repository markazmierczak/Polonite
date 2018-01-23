// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_BUFFEREDSTREAM_H_
#define STP_BASE_IO_BUFFEREDSTREAM_H_

#include "Base/Io/Stream.h"
#include "Base/Mem/OwnPtr.h"

namespace stp {

// NOTE: For mixed read/write operation the underlying stream is required to
//       be seekable.
class BASE_EXPORT BufferedStream final : public Stream {
 public:
  static constexpr int DefaultBufferSize = 4096;

  BufferedStream();
  ~BufferedStream() override;

  ALWAYS_INLINE Stream* GetUnderlying() const { return underlying_; }

  // The |new_size| must be greater than zero.
  // The size of buffer can be changed only when buffers are flushed.
  // Otherwise the behavior is undefined.
  void SetBufferSize(int new_size);
  ALWAYS_INLINE int GetBufferSize() const { return buffer_size_; }

  void FlushBuffers();

  void Open(OwnPtr<Stream> underlying) { OpenInternal(underlying.Release(), true); }
  void Open(Ptr<Stream> underlying) { OpenInternal(ToPointer(underlying), false); }

  void Close() override;
  bool IsOpen() const noexcept override;
  int ReadAtMost(MutableBufferSpan output) override;
  void Write(BufferSpan input) override;
  void WriteByte(byte_t byte) override;
  int TryReadByte() override;
  int64_t Seek(int64_t offset, SeekOrigin origin) override;
  void Flush() override;
  bool CanRead() override;
  bool CanWrite() override;
  bool CanSeek() override;
  void SetLength(int64_t length) override;
  int64_t GetLength() override;
  void SetPosition(int64_t position) override;
  int64_t GetPosition() override;

 private:
  void OpenInternal(Stream* underlying, bool owned);

  bool HasPendingWrite() const { return write_pos_ > 0; }
  bool HasPendingRead() const { return read_pos_ < read_len_; }

  void FlushWriteBuffer();
  void FlushReadBuffer();

  void ClearReadBufferBeforeWrite();

  int ReadFromBuffer(MutableBufferSpan output);
  int WriteToBuffer(BufferSpan input);

  void EnsureBufferAllocated();

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

  DISALLOW_COPY_AND_ASSIGN(BufferedStream);
};

} // namespace stp

#endif // STP_BASE_IO_BUFFEREDSTREAM_H_
