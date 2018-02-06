// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncoding.h"

#include "Base/Compiler/ByteOrder.h"
#include "Base/Text/Utf.h"

namespace stp {

namespace {

template<ByteOrder TOrder>
inline char32_t DecodeOne(const byte_t* b) {
  if (TOrder == ByteOrder::LittleEndian)
    return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
  else
    return b[3] | (b[2] << 8) | (b[1] << 16) | (b[0] << 24);
}

template<ByteOrder TOrder, typename T>
inline TextConversionResult DecodeTmpl(
    TextConversionContext* context, BufferSpan input, MutableSpan<T> output, bool flush) {
  auto* input_data = static_cast<const byte_t*>(input.data());
  int num_read = 0;

  int input_size = input.size();
  int leftover = input_size & 3;
  input_size &= ~3;

  int num_wrote = 0;

  bool did_fallback = false;

  while (num_read < input_size && num_wrote < output.size()) {
    char32_t c = DecodeOne<TOrder>(input_data + num_read);
    if (!unicode::IsValidCodepoint(c)) {
      c = unicode::ReplacementCodepoint;
      did_fallback = true;
    }
    int encoded = TryEncodeUtf(output.GetSlice(num_wrote), c);
    if (encoded == 0)
      break;
    num_wrote += encoded;
  }
  if (flush && leftover != 0) {
    int encoded = TryEncodeUtf(output.GetSlice(num_wrote), unicode::ReplacementCodepoint);
    if (encoded != 0) {
      did_fallback = true;
      num_read += leftover;
      num_wrote += encoded;
    }
  }
  return TextConversionResult(num_read, num_wrote, did_fallback);
}

TextConversionResult DecodeLE(
    TextConversionContext* context, BufferSpan input, MutableStringSpan output, bool flush) {
  return DecodeTmpl<ByteOrder::LittleEndian>(context, input, output, flush);
}
TextConversionResult Decode16LE(
    TextConversionContext* context, BufferSpan input, MutableString16Span output, bool flush) {
  return DecodeTmpl<ByteOrder::LittleEndian>(context, input, output, flush);
}
TextConversionResult DecodeBE(
    TextConversionContext* context, BufferSpan input, MutableStringSpan output, bool flush) {
  return DecodeTmpl<ByteOrder::BigEndian>(context, input, output, flush);
}
TextConversionResult Decode16BE(
    TextConversionContext* context, BufferSpan input, MutableString16Span output, bool flush) {
  return DecodeTmpl<ByteOrder::BigEndian>(context, input, output, flush);
}

template<ByteOrder TOrder>
inline int EncodeOne(char32_t c, byte_t* out) {
  if (TOrder == ByteOrder::LittleEndian) {
    out[0] = static_cast<byte_t>((c >> 0) & 0xFF);
    out[1] = static_cast<byte_t>((c >> 8) & 0xFF);
    out[2] = static_cast<byte_t>((c >> 16) & 0xFF);
    out[3] = static_cast<byte_t>((c >> 24) & 0xFF);
  } else {
    out[3] = static_cast<byte_t>((c >> 0) & 0xFF);
    out[2] = static_cast<byte_t>((c >> 8) & 0xFF);
    out[1] = static_cast<byte_t>((c >> 16) & 0xFF);
    out[0] = static_cast<byte_t>((c >> 24) & 0xFF);
  }
  return 4;
}

template<ByteOrder TOrder, typename T>
inline TextConversionResult EncodeTmpl(
    TextConversionContext* context, Span<T> input, MutableBufferSpan output) {
  auto* iptr = begin(input);
  auto* const iptr_end = end(input);
  auto* output_data = static_cast<byte_t*>(output.data());
  int num_wrote = 0;
  bool did_fallback = false;

  int max_output = output.size() - isizeof(char32_t);
  while (iptr < iptr_end && num_wrote <= max_output) {
    char32_t c = DecodeUtf(iptr, iptr_end);
    if (!UtfBase::IsDecodeError(c)) {
      num_wrote += EncodeOne<TOrder>(c, output_data + num_wrote);
    } else {
      num_wrote += EncodeOne<TOrder>(unicode::ReplacementCodepoint, output_data + num_wrote);
      did_fallback = true;
    }
  }
  return TextConversionResult(iptr - input.data(), num_wrote, did_fallback);
}

TextConversionResult EncodeLE(
    TextConversionContext* context, StringSpan input, MutableBufferSpan output) {
  return EncodeTmpl<ByteOrder::LittleEndian>(context, input, output);
}
TextConversionResult Encode16LE(
    TextConversionContext* context, String16Span input, MutableBufferSpan output) {
  return EncodeTmpl<ByteOrder::LittleEndian>(context, input, output);
}
TextConversionResult EncodeBE(
    TextConversionContext* context, StringSpan input, MutableBufferSpan output) {
  return EncodeTmpl<ByteOrder::BigEndian>(context, input, output);
}
TextConversionResult Encode16BE(
    TextConversionContext* context, String16Span input, MutableBufferSpan output) {
  return EncodeTmpl<ByteOrder::BigEndian>(context, input, output);
}

constexpr TextCodecVtable VtableLE = {
  DecodeLE, Decode16LE,
  EncodeLE, Encode16LE,
};

constexpr TextCodecVtable VtableBE = {
  DecodeBE, Decode16BE,
  EncodeBE, Encode16BE,
};

constexpr auto BuildLE() {
  auto builder = BuildTextCodec("UTF-32LE", VtableLE);
  builder.SetIanaCodepage(1019);
  builder.SetWindowsCodepage(12000);
  return builder;
}

constexpr auto BuildBE() {
  auto builder = BuildTextCodec("UTF-32BE", VtableBE);
  builder.SetIanaCodepage(1018);
  builder.SetWindowsCodepage(12001);
  return builder;
}

constexpr auto Build() {
  auto builder = BuildTextCodec("UTF-32", VtableBE);
  builder.SetIanaCodepage(1017);
  builder.SetWindowsCodepage(12000);
  return builder;
}

} // namespace

namespace detail {
constexpr const TextCodec Utf32LECodec = BuildLE();
constexpr const TextCodec Utf32BECodec = BuildBE();
constexpr const TextCodec Utf32Codec = Build();
} // namespace detail

} // namespace stp
