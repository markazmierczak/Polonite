// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_MEMORYSTREAM_H_
#define STP_BASE_IO_MEMORYSTREAM_H_

#include "Base/Containers/Buffer.h"
#include "Base/Io/Stream.h"
#include "Base/Type/Limits.h"

namespace stp {

class MemoryStream;

// Can handle 1GB memory at most.
class BASE_EXPORT MemoryStream final : public Stream {
 public:
  MemoryStream();
  ~MemoryStream() override;

  void OpenNewBytes();
  void AdoptAndOpen(Buffer&& bytes);
  Buffer CloseAndrelease();

  void Open(BufferSpan memory) { OpenInternal(const_cast<void*>(memory.data()), memory.size(), false); }
  void Open(MutableBufferSpan memory) { OpenInternal(memory.data(), memory.size(), true); }

  void EnsureCapacity(int request);

  void Close() override;
  bool IsOpen() const noexcept override;
  int ReadAtMost(MutableBufferSpan output) override;
  void Write(BufferSpan input) override;
  void PositionalRead(int64_t offset, MutableBufferSpan output) override;
  void PositionalWrite(int64_t offset, BufferSpan input) override;
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
  // Memory buffer allocated with malloc().
  byte_t* memory_ = nullptr;
  // Number of bytes within memory stream.
  int length_ = 0;
  int capacity_ = -1;
  // Current head for read/write operations.
  int position_ = 0;

  bool writable_ = false;
  bool expandable_ = false;
  bool open_ = false;

  static constexpr int MinCapacity_ = 4096;
  // The capacity computations relies strictly on this value.
  // If it would be greater, integer overflows.
  static constexpr int MaxCapacity_ = Limits<int>::Min / -2;

  void OpenInternal(void* data, int length, bool writable);
};

} // namespace stp

#endif // STP_BASE_IO_MEMORYSTREAM_H_
