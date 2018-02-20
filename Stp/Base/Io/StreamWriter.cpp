// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/StreamWriter.h"

#include "Base/Compiler/Endianness.h"
#include "Base/Io/Stream.h"
#include "Base/Math/Alignment.h"
#include "Base/Memory/PolymorphicAllocator.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/Codec/Utf8Encoding.h"
#include "Base/Text/Utf.h"

namespace stp {

StreamWriter::StreamWriter(Stream* stream)
    : StreamWriter(stream, BuiltinTextEncodings::Utf8()) {
}

StreamWriter::StreamWriter(Stream* stream, TextEncoding encoding)
    : StreamWriter(stream, encoding, MinBufferCapacity) {
}

StreamWriter::StreamWriter(Stream* stream, TextEncoding encoding, int buffer_capacity)
    : stream_(*stream),
      encoding_(encoding) {
  ASSERT(encoding_.CanEncode());

  if (encoding_ != BuiltinTextEncodings::Utf8()) {
    CreateEncoder();
  }

  // Compute buffer capacity and align to maximum character size.
  if (buffer_capacity < MinBufferCapacity) {
    buffer_capacity = MinBufferCapacity;
  } else {
    buffer_capacity = alignForward(buffer_capacity, isizeof(char32_t));
  }
  buffer_.ensureCapacity(buffer_capacity);
}

StreamWriter::~StreamWriter() {
  if (encoder_)
    DestroyEncoder();
}

TextEncoding StreamWriter::GetEncoding() const {
  return encoding_;
}

void StreamWriter::ForceValidation() {
  if (!encoder_)
    CreateEncoder();
}

namespace {

class SimpleBufferAllocator : public PolymorphicAllocator {
 public:
  explicit SimpleBufferAllocator(Buffer& buffer) : buffer_(buffer) {}

  void* Allocate(int size) override { return buffer_.appendUninitialized(size); }
  void* Reallocate(void* ptr, int old_size, int new_size) { ASSERT(false); return nullptr; }
  void Deallocate(void* ptr, int size) { ASSERT(false); }

 private:
  Buffer& buffer_;
};

} // namespace

void StreamWriter::CreateEncoder() {
  ASSERT(!encoder_);
  SimpleBufferAllocator allocator(encoder_memory_);
  encoder_ = encoding_.CreateEncoder(allocator);
}

void StreamWriter::DestroyEncoder() {
  encoder_->~TextEncoder();
}

void StreamWriter::onFlush() {
  if (buffer_.isEmpty())
    return;
  FlushBuffer();
  stream_.Flush();
}

void StreamWriter::FlushBuffer() {
  if (buffer_.isEmpty())
    return;
  stream_.Write(buffer_);
  buffer_.clear();
}

void StreamWriter::SetAutoFlush(bool auto_flush) {
  if (auto_flush_ == auto_flush)
    return;
  auto_flush_ = auto_flush;
  if (auto_flush_)
    FlushBuffer();
}

void StreamWriter::WriteToBuffer(BufferSpan input) {
  if (UNLIKELY(auto_flush_)) {
    stream_.Write(input);
    return;
  }

  int remaining_capacity = buffer_.capacity() - buffer_.size();

  if (input.size() <= remaining_capacity) {
    buffer_.append(input);
    return;
  }
  if (input.size() >= buffer_.capacity() + remaining_capacity) {
    FlushBuffer();
    stream_.Write(input);
    return;
  }

  buffer_.append(input.getSlice(remaining_capacity));
  input.removePrefix(remaining_capacity);

  FlushBuffer();

  ASSERT(input.size() < buffer_.capacity());
  buffer_.append(input);
}

void StreamWriter::onWriteChar(char c) {
  if (IsDirect()) {
    if (buffer_.capacity() == buffer_.size()) {
      FlushBuffer();
    }
    buffer_.add(static_cast<byte_t>(c));
    return;
  }
  WriteIndirect(StringSpan(&c, 1));
}

void StreamWriter::onWriteRune(char32_t rune) {
  char units[Utf8::MaxEncodedRuneLength];
  int length = Utf8::Encode(units, rune);
  if (IsDirect()) {
    WriteToBuffer(BufferSpan(units, length));
  } else {
    WriteIndirect(StringSpan(units, length));
  }
}

void StreamWriter::onWriteString(StringSpan input) {
  if (IsDirect()) {
    WriteToBuffer(BufferSpan(input));
  } else {
    WriteIndirect(input);
  }
}

void StreamWriter::WriteIndirect(StringSpan input) {
  while (!input.isEmpty()) {
    int remaining_capacity = buffer_.capacity() - buffer_.size();
    void* output = buffer_.appendUninitialized(remaining_capacity);

    auto result = encoder_->Encode(input, MutableBufferSpan(output, remaining_capacity));

    input.removePrefix(result.num_read);
    buffer_.removeSuffix(remaining_capacity - result.num_wrote);

    if (result.more_output) {
      FlushBuffer();
    }
  }
}

} // namespace stp
