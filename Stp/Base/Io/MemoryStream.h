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

  void openNewBytes();
  void adoptAndOpen(Buffer&& bytes);
  Buffer closeAndrelease();

  void open(BufferSpan memory) { openInternal(const_cast<void*>(memory.data()), memory.size(), false); }
  void open(MutableBufferSpan memory) { openInternal(memory.data(), memory.size(), true); }

  void ensureCapacity(int request);

  void close() override;
  bool isOpen() const noexcept override;
  int readAtMost(MutableBufferSpan output) override;
  void write(BufferSpan input) override;
  void positionalRead(int64_t offset, MutableBufferSpan output) override;
  void positionalWrite(int64_t offset, BufferSpan input) override;
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

  void openInternal(void* data, int length, bool writable);
};

} // namespace stp

#endif // STP_BASE_IO_MEMORYSTREAM_H_
