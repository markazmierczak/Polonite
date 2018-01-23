// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/StreamWriter.h"

#include "Base/Compiler/ByteOrder.h"
#include "Base/Math/Alignment.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/Codec/AsciiCodec.h"
#include "Base/Text/Codec/Utf16Codec.h"
#include "Base/Text/Codec/Utf32Codec.h"
#include "Base/Text/Codec/Utf8Codec.h"
#include "Base/Text/Utf.h"

namespace stp {

StreamWriter::StreamWriter(Stream* stream)
    : StreamWriter(stream, Utf8Codec()) {
}

StreamWriter::StreamWriter(Stream* stream, TextCodec codec)
    : StreamWriter(stream, codec, MinBufferCapacity) {
}

StreamWriter::StreamWriter(Stream* stream, TextCodec codec, int buffer_capacity)
    : stream_(stream) {
  // Compute buffer capacity and align to maximum character size.
  if (buffer_capacity < MinBufferCapacity)
    buffer_capacity = MinBufferCapacity;

  buffer_capacity = AlignForward(buffer_capacity, isizeof(char32_t));

  buffer_.EnsureCapacity(buffer_capacity);

  encoder_.codec = codec;
  encoder_.context = nullptr;

  codec_class_ = SelectCodecClass(codec);
}

StreamWriter::~StreamWriter() {
  if (IsTranscoderInitialized()) {
    encoder_.Cleanup();
    decoder_.Cleanup();
  }
}

TextCodec StreamWriter::GetEncoding() const {
  return encoder_.codec;
}

StreamWriter::CodecClass StreamWriter::SelectCodecClass(TextCodec codec) {
  static_assert(ByteOrder::Native == ByteOrder::LittleEndian, "!");

  switch (codec.GetIanaCodepage()) {
    case Utf8Codec::IanaCodepage:
      return CodecClass::Utf8;
    case Utf16LECodec::IanaCodepage:
      return CodecClass::Utf16;
    case Utf32LECodec::IanaCodepage:
      return CodecClass::Utf32;
  }
  if (codec.IsSingleByteAsciiCompatible())
    return CodecClass::AsciiCompatbile;
  return CodecClass::None;
}

void StreamWriter::OnFlush() {
  FlushBuffer();
  stream_->Flush();
}

void StreamWriter::FlushBuffer() {
  if (!buffer_.IsEmpty()) {
    WriteToStream(buffer_.data(), buffer_.size());
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

void StreamWriter::WriteToStream(const byte_t* bytes, int size) {
  stream_->Write(bytes, size);
}

void StreamWriter::WriteBytesInternal(const byte_t* bytes, int size) {
  if (UNLIKELY(auto_flush_)) {
    WriteToStream(bytes, size);
    return;
  }

  int remaining_capacity = buffer_.GetRemainingCapacity();

  if (size <= remaining_capacity) {
    buffer_.Append(bytes, size);
    return;
  }
  if (size >= buffer_.capacity() + remaining_capacity) {
    FlushBuffer();
    WriteToStream(bytes, size);
    return;
  }

  buffer_.Append(bytes, remaining_capacity);
  FlushBuffer();

  bytes += remaining_capacity;
  size -= remaining_capacity;
  ASSERT(size < buffer_.capacity());
  buffer_.Append(bytes, size);
}

void StreamWriter::OnWriteBytes(const byte_t* bytes, int size) {
  WriteBytesInternal(bytes, size);
}

void StreamWriter::OnWriteAsciiChar(char c) {
  ASSERT(IsAscii(c));

  byte_t bytes[4] = { static_cast<byte_t>(c), 0, 0, 0 };

  switch (codec_class_) {
    case CodecClass::Utf8:
    case CodecClass::AsciiCompatbile:
      WriteBytesInternal(bytes, 1);
      break;
    case CodecClass::Utf16:
      WriteBytesInternal(bytes, 2);
      break;
    case CodecClass::Utf32:
      WriteBytesInternal(bytes, 4);
      break;
    case CodecClass::None: {
      auto uc = char_cast<char32_t>(c);
      WriteWithEncoder(&uc, 1);
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
    case CodecClass::Utf32:
      WriteBytesInternal(reinterpret_cast<const byte_t*>(&codepoint), 4);
      break;
    case CodecClass::AsciiCompatbile:
    case CodecClass::None: {
      WriteWithEncoder(&codepoint, 1);
      break;
    }
  }
}

void StreamWriter::WriteUnicodeCharAsUtf8(char32_t codepoint) {
  char code_units[Utf8::MaxEncodedCodepointLength];
  int length = Utf8::Encode(code_units, codepoint);
  WriteBytesInternal(reinterpret_cast<const byte_t*>(code_units), length);
}

void StreamWriter::WriteUnicodeCharAsUtf16(char32_t codepoint) {
  char16_t code_units[Utf16::MaxEncodedCodepointLength];
  int length = Utf16::Encode(code_units, codepoint);
  WriteBytesInternal(reinterpret_cast<const byte_t*>(code_units), length << 1);
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

void StreamWriter::OnWriteAscii(const char* text, int length) {
  switch (codec_class_) {
    case CodecClass::Utf8:
    case CodecClass::AsciiCompatbile:
      WriteBytesInternal(reinterpret_cast<const byte_t*>(text), length);
      break;
    case CodecClass::Utf16:
      OnWriteAsciiTmpl<char16_t>(text, length);
      break;
    case CodecClass::Utf32:
      OnWriteAsciiTmpl<char32_t>(text, length);
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

void StreamWriter::OnWriteUtf8(const char* text, int length) {
  switch (codec_class_) {
    case CodecClass::Utf8:
      WriteBytesInternal(reinterpret_cast<const byte_t*>(text), length);
      break;
    case CodecClass::Utf16:
      ConvertAsciiOrUnicode<char16_t>(text, length);
      break;
    case CodecClass::Utf32:
      ConvertAsciiOrUnicode<char32_t>(text, length);
      break;
    case CodecClass::AsciiCompatbile:
      if (StringSpan(text, length).IsAscii()) {
        OnWriteAscii(text, length);
        break;
      }
      // no break
    case CodecClass::None:
      Convert(reinterpret_cast<const byte_t*>(text), length, Utf8Codec());
      break;
  }
}

void StreamWriter::OnWriteUtf16(const char16_t* text, int length) {
  switch (codec_class_) {
    case CodecClass::Utf8:
      ConvertAsciiOrUnicode<char>(text, length);
      break;
    case CodecClass::Utf16:
      WriteBytesInternal(reinterpret_cast<const byte_t*>(text), length << 1);
      break;
    case CodecClass::Utf32:
      ConvertAsciiOrUnicode<char32_t>(text, length);
      break;
    case CodecClass::AsciiCompatbile:
    case CodecClass::None:
      Convert(reinterpret_cast<const byte_t*>(text), length << 1, Utf16LECodec());
      break;
  }
}

void StreamWriter::OnWriteUtf32(const char32_t* text, int length) {
  switch (codec_class_) {
    case CodecClass::Utf8:
      ConvertAsciiOrUnicode<char>(text, length);
      break;
    case CodecClass::Utf16:
      ConvertAsciiOrUnicode<char16_t>(text, length);
      break;
    case CodecClass::Utf32:
      WriteBytesInternal(reinterpret_cast<const byte_t*>(text), length << 2);
      break;
    case CodecClass::AsciiCompatbile:
    case CodecClass::None:
      WriteWithEncoder(text, length);
      break;
  }
}

void StreamWriter::OnWriteCustom(const byte_t* bytes, int size, TextCodec encoding) {
  if (encoder_.codec == encoding)
    OnWriteBytes(bytes, size);
  else
    Convert(bytes, size, encoding);
}

void StreamWriter::InitTranscoder() {
  encoder_.Init(this);
  decoder_.Init(this);

  encoder_.on_grow = &EncoderOnGrow;
}

TextCodecError StreamWriter::EncoderOnGrow(TextEncoder& encoder, int input_size) {
  auto* that = static_cast<StreamWriter*>(encoder.context);
  that->EndWrite();
  that->BeginWrite();
  return TextCodecError::TryEgain;
}

void StreamWriter::BeginWrite() {
  if (buffer_.GetRemainingCapacity() < 16)
    FlushBuffer();

  encoder_.count = buffer_.GetRemainingCapacity();
  encoder_.output = buffer_.AppendUninitialized(encoder_.count);
  encoder_.index = 0;
}

void StreamWriter::EndWrite() {
  int unused_count = encoder_.count - encoder_.index;
  buffer_.RemoveSuffix(unused_count);

  encoder_.index = 0;
  encoder_.count = 0;
}

void StreamWriter::WriteWithEncoder(const char32_t* text, int length) {
  if (!IsTranscoderInitialized())
    InitTranscoder();

  BeginWrite();
  TextCodec::WriteCharacters(encoder_, text, length);
  EndWrite();

  if (auto_flush_)
    FlushBuffer();
}

void StreamWriter::SwitchDecoder(TextCodec encoding) {
  if (decoder_.codec == encoding)
    return;

  if (!IsTranscoderInitialized())
    InitTranscoder();
  else
    decoder_.Cleanup();

  decoder_.codec = encoding;
}

void StreamWriter::Convert(const byte_t* bytes, int size, TextCodec encoding) {
  // It might be really slow operation to replace the decoder.
  // Check if we are going to write anything after all.
  if (size <= 0)
    return;

  SwitchDecoder(encoding);

  decoder_.input = bytes;
  decoder_.count = size;
  decoder_.index = 0;

  BeginWrite();
  auto error = TextCodec::Convert(decoder_, encoder_);
  EndWrite();

  if (auto_flush_)
    FlushBuffer();

  if (error != TextCodecError::Ok)
    OnCodecError(error);
}

void StreamWriter::OnCodecError(TextCodecError error) {
  // FIXME implement
}

} // namespace stp
