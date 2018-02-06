// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncoding.h"

#include "Base/Text/Utf.h"

namespace stp {

namespace {

// 0x80 - 0x9F
const char16_t Cp1252ToUnicode[32] = {
  0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
  0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x017D, 0x0000,
  0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
  0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x017E, 0x0178,
};

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
      output[num_wrote++] = static_cast<char>(b);
    } else {
      char16_t c;
      if (b >= 0xA0) {
        c = static_cast<char16_t>(b);
      } else {
        c = Cp1252ToUnicode[b - 0x80];
        if (!c) {
          c = unicode::ReplacementCodepoint;
          did_fallback = true;
        }
      }
      int encoded = TryEncodeUtf(output.GetSlice(num_wrote), c);
      if (encoded == 0)
        break;
      num_wrote += encoded;
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

// 0x0150..0x0197
const byte_t Cp1252Page01[72] = {
  0x00, 0x00, 0x8C, 0x9C, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x8A, 0x9A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x9F, 0x00, 0x00, 0x00, 0x00, 0x8E, 0x9E, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00,
};
// 0x02C0..0x02DF
const byte_t Cp1252Page02[32] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x98, 0x00, 0x00, 0x00,
};
// 0x2010..0x203F
const byte_t Cp1252Page20[48] = {
  0x00, 0x00, 0x00, 0x96, 0x97, 0x00, 0x00, 0x00,
  0x91, 0x92, 0x82, 0x00, 0x93, 0x94, 0x84, 0x00,
  0x86, 0x87, 0x95, 0x00, 0x00, 0x00, 0x85, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x8B, 0x9B, 0x00, 0x00, 0x00, 0x00, 0x00,
};

byte_t EncodeExtra(char32_t c) {
  ASSERT(!IsAscii(c));
  ASSERT(!(0x00A0 <= c && c < 0x0100));

  if (0x0150 <= c && c < 0x0198)
    return Cp1252Page01[c - 0x0150];
  if (0x02C0 <= c && c < 0x02E0)
    return Cp1252Page02[c - 0x02C0];
  if (0x2010 <= c && c < 0x2040)
    return Cp1252Page20[c - 0x2010];
  if (c == 0x20AC)
    return 0x80;
  if (c == 0x2122)
    return 0x99;
  return 0;
}

template<typename T>
inline TextConversionResult EncodeTmpl(
    TextConversionContext* context, Span<T> input, MutableBufferSpan output) {
  auto* iptr = begin(input);
  auto* const iptr_end = end(input);
  int num_wrote = 0;
  auto* optr = static_cast<byte_t*>(output.data());
  bool did_fallback = false;

  while (iptr < iptr_end) {
    char32_t c = DecodeUtf(iptr, iptr_end);
    if ((c < 0x80) || (0x00A0 <= c && c < 0x0100)) {
      optr[num_wrote++] = static_cast<byte_t>(c);
    } else {
      byte_t b = EncodeExtra(c);
      if (b != 0) {
        optr[num_wrote++] = b;
      } else {
        optr[num_wrote++] = '?';
        did_fallback = true;
      }
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
  "windows-1252",
};

constexpr TextCodecVtable Vtable = {
  Decode, Decode16,
  Encode, Encode16,
};

constexpr auto Build() {
  auto builder = BuildTextCodec("cp1252", Vtable);
  builder.SetAliases(Aliases);
  builder.SetIanaCodepage(2252);
  builder.SetWindowsCodepage(1252);
  return builder;
}

} // namespace

namespace detail {
constexpr const TextCodec Cp1252Codec = Build();
}

} // namespace stp
