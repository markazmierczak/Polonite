// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncoding.h"

#include "Base/Text/Utf.h"

namespace stp {

namespace {

TextDecoder::Result Decode(
    TextConversionContext* context, BufferSpan input, MutableStringSpan output, bool flush) {
  auto* input_data = static_cast<const byte_t*>(input.data());
  int num_read = 0;
  int num_wrote = 0;

  while (num_read < input.size() && num_wrote < output.size()) {
    byte_t b = input_data[num_read];
    if (LIKELY(b < 0x80)) {
      output[num_wrote++] = static_cast<char>(b);
    } else {
      if (output.size() - num_wrote < 2)
        break;
      num_wrote += Utf8::EncodeInTwoUnits(output.data() + num_wrote, static_cast<char16_t>(b));
    }
    ++num_read;
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
    if (UNLIKELY(c >= 0x100)) {
      c = '?';
      did_fallback = true;
    }
    output_data[num_wrote++] = static_cast<byte_t>(c);
  }
  return TextConversionResult(iptr - input.data(), num_wrote, did_fallback);
}

constexpr StringSpan Aliases[] = {
  "iso-ir-100",
  "latin1",
  "L1",
  "IBM819",
  "CP819",
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
