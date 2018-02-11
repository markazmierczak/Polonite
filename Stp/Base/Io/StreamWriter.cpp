// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/StreamWriter.h"

#include "Base/Compiler/ByteOrder.h"
#include "Base/Math/Alignment.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/Utf.h"

namespace stp {

StreamWriter::StreamWriter(Stream* stream)
    : StreamWriter(stream, TextEncoding::BuiltinUtf8()) {
}

StreamWriter::StreamWriter(Stream* stream, TextEncoding encoding)
    : StreamWriter(stream, encoding, MinBufferCapacity) {
}

StreamWriter::StreamWriter(Stream* stream, TextEncoding encoding, int buffer_capacity)
    : stream_(stream),
      encoding_(encoding),
      encoder_(encoding) {
  ASSERT(stream_ != nullptr);
  ASSERT(encoding_.CanDecode());
  // Compute buffer capacity and align to maximum character size.
  if (buffer_capacity < MinBufferCapacity)
    buffer_capacity = MinBufferCapacity;
  else
    buffer_capacity = AlignForward(buffer_capacity, isizeof(char32_t));

  buffer_.EnsureCapacity(buffer_capacity);

  codec_class_ = SelectCodecClass(encoding_);
}

StreamWriter::~StreamWriter() {}

TextEncoding StreamWriter::GetEncoding() const {
  return encoding_;
}

StreamWriter::CodecClass StreamWriter::SelectCodecClass(TextEncoding encoding) {
  static_assert(ByteOrder::Native == ByteOrder::LittleEndian, "!");
  if (encoding == TextEncoding::BuiltinUtf8())
    return CodecClass::Utf8;
  if (encoding == TextEncoding::BuiltinUtf16())
    return CodecClass::Utf16;
  return CodecClass::None;
}

void StreamWriter::OnFlush() {
  FlushBuffer();
  stream_->Flush();
}

void StreamWriter::FlushBuffer() {
  if (!buffer_.IsEmpty()) {
    stream_->Write(buffer_);
    buffer_.Clear();
  }
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
    stream_->Write(input);
    return;
  }

  int remaining_capacity = buffer_.capacity() - buffer_.size();

  if (input.size() <= remaining_capacity) {
    buffer_.Append(input);
    return;
  }
  if (input.size() >= buffer_.capacity() + remaining_capacity) {
    FlushBuffer();
    stream_->Write(input);
    return;
  }

  buffer_.Append(input.GetSlice(remaining_capacity));
  input.RemovePrefix(remaining_capacity);

  FlushBuffer();

  ASSERT(input.size() < buffer_.capacity());
  buffer_.Append(input);
}

void StreamWriter::OnWriteAsciiChar(char c) {
  ASSERT(IsAscii(c));

  byte_t bytes[4] = { static_cast<byte_t>(c), 0, 0, 0 };

  switch (codec_class_) {
    case CodecClass::Utf8:
      WriteToBuffer(BufferSpan(bytes, 1));
      break;
    case CodecClass::Utf16:
      WriteToBuffer(BufferSpan(bytes, 2));
      break;
    case CodecClass::None: {
      TranscodeChar(char_cast<char32_t>(c));
      break;
    }
  }
}

void StreamWriter::OnWriteUnicodeChar(char32_t codepoint) {
  ASSERT(Unicode::IsValidCodepoint(codepoint));

  if (IsAscii(codepoint)) {
    OnWriteAsciiChar(static_cast<char>(codepoint));
    return;
  }

  switch (codec_class_) {
    case CodecClass::Utf8:
      WriteUnicodeCharAsUtf8(codepoint);
      break;
    case CodecClass::Utf16:
      WriteUnicodeCharAsUtf16(codepoint);
      break;
    case CodecClass::None:
      TranscodeChar(codepoint);
      break;
  }
}

void StreamWriter::WriteUnicodeCharAsUtf8(char32_t codepoint) {
  char code_units[Utf8::MaxEncodedCodepointLength];
  int length = Utf8::Encode(code_units, codepoint);
  WriteToBuffer(BufferSpan(code_units, length));
}

void StreamWriter::WriteUnicodeCharAsUtf16(char32_t codepoint) {
  char16_t code_units[Utf16::MaxEncodedCodepointLength];
  int length = Utf16::Encode(code_units, codepoint);
  WriteToBuffer(BufferSpan(code_units, length));
}

// Shift amount used to convert between number of bytes and number of characters.
template<typename TChar>
static constexpr int ShiftForCharacter = isizeof(TChar) / 2;

template<typename TDst, typename TSrc>
static void CopyAsciiToBuffer(Bytes& buffer, const TSrc* text, int length) {
  constexpr int Shift = ShiftForCharacter<TDst>;
  byte_t* dst_bytes = buffer.AppendUninitialized(length << Shift);
  auto* dst = reinterpret_cast<TDst*>(dst_bytes);
  for (int i = 0; i < length; ++i)
    dst[i] = text[i];
}

template<typename TDst, typename TSrc>
void StreamWriter::OnWriteAsciiTmpl(const TSrc* text, int length) {
  constexpr int Shift = ShiftForCharacter<TDst>;
  int capacity = buffer_.GetRemainingCapacity() >> Shift;
  int part_length = Min(length, capacity);
  CopyAsciiToBuffer<TDst>(buffer_, text, part_length);

  text += part_length;
  length -= part_length;

  if (length > 0)
    capacity = buffer_.capacity() >> Shift;

  while (length > 0) {
    FlushBuffer();

    part_length = Min(length, capacity);
    CopyAsciiToBuffer<TDst>(buffer_, text, part_length);

    text += part_length;
    length -= part_length;
  }
}

void StreamWriter::OnWriteAscii(StringSpan input) {
  switch (codec_class_) {
    case CodecClass::Utf8:
      WriteToBuffer(BufferSpan(input));
      break;
    case CodecClass::Utf16:
      OnWriteAsciiTmpl<char16_t>(input);
      break;
    case CodecClass::None:
      Convert(reinterpret_cast<const byte_t*>(text), length, AsciiCodec());
      break;
  }
}

template<typename TDst, typename TSrc>
void StreamWriter::ConvertUnicode(const TSrc* text, int length) {
  constexpr int Shift = ShiftForCharacter<TDst>;

  const TSrc* src = text;
  const TSrc* src_end = text + length;

  int remaining = buffer_.GetRemainingCapacity();
  byte_t* dst = buffer_.AppendUninitialized(remaining);
  ASSERT(IsAlignedTo(dst, sizeof(TDst)));

  while (src < src_end) {
    // Decode single code-point.
    char32_t codepoint = UtfTmpl<TSrc>::Decode(src, src_end);
    if (UNLIKELY(Unicode::IsDecodeError(codepoint))) {
      // FIXME call replacement handler
      codepoint = Unicode::ReplacementCharacter;
    }

    // Encode single code-point.
    constexpr int MaxEncodedSizeInBytes = UtfTmpl<TDst>::MaxEncodedCodepointLength << Shift;
    if (UNLIKELY(remaining < MaxEncodedSizeInBytes)) {
      FlushBuffer();
      remaining = buffer_.capacity();
      dst = buffer_.AppendUninitialized(remaining);
    }

    int n = UtfTmpl<TDst>::Encode(reinterpret_cast<TDst*>(dst), codepoint);
    n <<= Shift;

    ASSERT(n <= MaxEncodedSizeInBytes);
    dst += n;
    remaining -= n;
  }

  buffer_.RemoveSuffix(remaining);
}

template<typename TDst, typename TSrc>
void StreamWriter::ConvertAsciiOrUnicode(const TSrc* text, int length) {
  if (StringSpanTmpl<const TSrc>(text, length).IsAscii())
    OnWriteAsciiTmpl<TDst>(text, length);
  else
    ConvertUnicode<TDst>(text, length);
}

void StreamWriter::OnWriteUtf8(StringSpan input) {
  switch (codec_class_) {
    case CodecClass::Utf8:
      WriteToBuffer(BufferSpan(input));
      break;
    case CodecClass::Utf16:
      WriteAsciiOrUnicode(input);
      break;
    case CodecClass::None:
      Convert(reinterpret_cast<const byte_t*>(text), length, Utf8Codec());
      break;
  }
}

void StreamWriter::OnWriteUtf16(String16Span input) {
  switch (codec_class_) {
    case CodecClass::Utf8:
      WriteAsciiOrUnicode(input);
      break;
    case CodecClass::Utf16:
      WriteToBuffer(BufferSpan(input));
      break;
    case CodecClass::None:
      Transcode(input, Utf16LECodec());
      break;
  }
}

} // namespace stp
