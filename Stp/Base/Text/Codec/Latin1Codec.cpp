// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncoding.h"

#include "Base/Text/Utf.h"

namespace stp {

namespace {

TextConversionResult Decode(
    TextConversionContext* context, BufferSpan input, MutableStringSpan output, bool flush) {
  auto* input_data = static_cast<const byte_t*>(input.data());
  auto* output_data = output.data();
  int num_read = 0;
  int num_wrote = 0;

  while (num_read < input.size() && num_wrote < output.size()) {
    byte_t b = input_data[num_read];
    if (b < 0x80) {
      output_data[num_wrote++] = static_cast<char>(b);
    } else {
      if (output.size() - num_wrote < 2)
        break;
      num_wrote += Utf8::EncodeInTwoUnits(output_data + num_wrote, static_cast<char16_t>(b));
    }
    ++num_read;
  }
  return TextConversionResult(num_read, num_wrote, false);
}

TextConversionResult Decode16(
    TextConversionContext* context, BufferSpan input, MutableString16Span output, bool flush) {
  auto* input_data = static_cast<const byte_t*>(input.data());
  auto* output_data = output.data();
  int num_read = 0;
  int num_wrote = 0;

  while (num_read < input.size() && num_wrote < output.size()) {
    output_data[num_wrote++] = static_cast<char16_t>(input_data[num_read++]);
  }
  return TextConversionResult(num_read, num_wrote, false);
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
    if (LIKELY(c < 0x100)) {
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
  "iso-ir-100",
  "latin1",
  "L1",
  "IBM819",
  "CP819",
};

constexpr TextCodecVtable Vtable = {
  Decode, Decode16,
  Encode, Encode16,
};

constexpr auto Build() {
  auto builder = BuildTextCodec("ISO-8859-1", Vtable);
  builder.SetAliases(Aliases);
  builder.SetIanaCodepage(4);
  builder.SetWindowsCodepage(28591);
  return builder;
}

} // namespace

namespace detail {
constexpr const TextCodec Latin1Codec = Build();
}

} // namespace stp
