// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncoding.h"

#include "Base/Text/Utf.h"

namespace stp {

namespace {

template<typename T>
inline TextConversionResult DecodeTmpl(
    TextConversionContext* context, BufferSpan input, MutableSpan<T> output, bool flush) {
  auto* input_data = static_cast<const byte_t*>(input.data());
  int num_read = 0;
  int num_wrote = 0;
  bool did_fallback = false;

  while (num_read < input.size() && num_wrote < output.size()) {
    byte_t b = input_data[num_read];
    if (LIKELY(b < 0x80)) {
      output[num_wrote++] = static_cast<T>(b);
    } else {
      int encoded = TryEncodeUtf(output.GetSlice(num_wrote), unicode::ReplacementCodepoint);
      if (encoded == 0)
        break;
      num_wrote += encoded;
      did_fallback = true;
    }
    ++num_read;
  }
  return TextConversionResult(num_read, num_wrote, did_fallback);
}

TextConversionResult Decode(
    TextConversionContext* context, BufferSpan input, MutableStringSpan output, bool flush) {
  return DecodeTmpl(context, input, output, flush);
}
TextConversionResult Decode16(
    TextConversionContext* context, BufferSpan input, MutableString16Span output, bool flush) {
  return DecodeTmpl(context, input, output, flush);
}

template<typename T>
inline TextConversionResult EncodeTmpl(
    TextConversionContext* context, Span<T> input, MutableBufferSpan output) {
  auto* iptr = begin(input);
  auto* const iptr_end = end(input);
  auto* output_data = static_cast<byte_t*>(output.data());
  int num_wrote = 0;
  bool did_fallback = false;

  while (iptr < iptr_end && num_wrote < output.size()) {
    char32_t c = DecodeUtf(iptr, iptr_end);
    if (LIKELY(IsAscii(c))) {
      output_data[num_wrote++] = static_cast<byte_t>(c);
    } else {
      output_data[num_wrote++] = '?';
      did_fallback = true;
    }
  }
  return TextConversionResult(iptr - input.data(), num_wrote, did_fallback);
}

TextConversionResult Encode(
    TextConversionContext* context, StringSpan input, MutableBufferSpan output) {
  return EncodeTmpl(context, input, output);
}
TextConversionResult Encode16(
    TextConversionContext* context, String16Span input, MutableBufferSpan output) {
  return EncodeTmpl(context, input, output);
}

constexpr StringSpan Aliases[] = {
  "ASCII",
  "iso-ir-6",
  "ANSI_X3.4-1968",
  "ANSI_X3.4-1986",
  "ISO_646.irv:1991",
  "ISO646-US",
  "us",
  "IBM367",
  "cp367",
};

constexpr TextCodecVtable Vtable = {
  Decode, Decode16,
  Encode, Encode16,
};

constexpr auto Build() {
  auto builder = BuildTextCodec("US-ASCII", Vtable);
  builder.SetAliases(Aliases);
  builder.SetIanaCodepage(3);
  builder.SetWindowsCodepage(20127);
  return builder;
}

} // namespace

namespace detail {
constexpr const TextCodec AsciiCodec = Build();
}

} // namespace stp
