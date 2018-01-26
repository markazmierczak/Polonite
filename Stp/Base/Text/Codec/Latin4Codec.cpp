// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Codec/TextCodec.h"

namespace stp {

namespace {

using Context = TextConversionContext;

// 0xA0..0xFF
const char16_t Latin4ToUnicode[96] = {
  0x00A0, 0x0104, 0x0138, 0x0156, 0x00A4, 0x0128, 0x013B, 0x00A7,
  0x00A8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00AD, 0x017D, 0x00AF,
  0x00B0, 0x0105, 0x02DB, 0x0157, 0x00B4, 0x0129, 0x013C, 0x02C7,
  0x00B8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014A, 0x017E, 0x014B,
  0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E,
  0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x012A,
  0x0110, 0x0145, 0x014C, 0x0136, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
  0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x0168, 0x016A, 0x00DF,
  0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F,
  0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x012B,
  0x0111, 0x0146, 0x014D, 0x0137, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
  0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x0169, 0x016B, 0x02D9,
};

int Decode(Context& context, BufferSpan input, MutableStringSpan output, bool flush) {
  auto* iptr = static_cast<const byte_t*>(input.data());
  const int isize = input.size();
  auto* optr = output.data();
  int oi = 0;

  for (int ii = 0; ii < isize; ++ii) {
    byte_t b = iptr[ii];
    if (b < 0x80) {
      optr[oi++] = static_cast<char>(b);
    } else {
      char16_t c = b < 0xA0 ? static_cast<char16_t>(b) : Latin4ToUnicode[b - 0xA0];
      oi += Utf8::EncodeInTwoUnits(optr + oi, c);
    }
  }
  ASSERT(oi <= output.size());
  return oi;
}

int Decode16(Context& context, BufferSpan input, MutableString16Span output, bool flush) {
  auto* iptr = static_cast<const byte_t*>(input.data());
  const int isize = input.size();
  auto* optr = output.data();
  int oi = 0;

  for (int ii = 0; ii < isize; ++ii) {
    byte_t b = iptr[ii];
    if (b < 0xA0)
      optr[oi++] = static_cast<char16_t>(b);
    else
      optr[oi++] = Latin4ToUnicode[b - 0xA0];
  }
  ASSERT(oi <= output.size());
  return oi;
}

int CountChars(const Context& context, BufferSpan input) {
  auto* iptr = static_cast<const byte_t*>(input.data());
  const int isize = input.size();
  int oi = 0;

  for (int i = 0; i < isize; ++i) {
    oi += iptr[i] < 0x80 ? 1 : 2;
  }
  return oi;
}

int CountChars16(const Context& context, BufferSpan input) {
  return input.size();
}

// 0x00A0..0x017F
const byte_t Latin4Page00[224] = {
  0xA0, 0x00, 0x00, 0x00, 0xA4, 0x00, 0x00, 0xA7,
  0xA8, 0x00, 0x00, 0x00, 0x00, 0xAD, 0x00, 0xAF,
  0xB0, 0x00, 0x00, 0x00, 0xB4, 0x00, 0x00, 0x00,
  0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0x00,
  0x00, 0xC9, 0x00, 0xCB, 0x00, 0xCD, 0xCE, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xD4, 0xD5, 0xD6, 0xD7,
  0xD8, 0x00, 0xDA, 0xDB, 0xDC, 0x00, 0x00, 0xDF,
  0x00, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0x00,
  0x00, 0xE9, 0x00, 0xEB, 0x00, 0xED, 0xEE, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xF4, 0xF5, 0xF6, 0xF7,
  0xF8, 0x00, 0xFA, 0xFB, 0xFC, 0x00, 0x00, 0x00,
  0xC0, 0xE0, 0x00, 0x00, 0xA1, 0xB1, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xC8, 0xE8, 0x00, 0x00,
  0xD0, 0xF0, 0xAA, 0xBA, 0x00, 0x00, 0xCC, 0xEC,
  0xCA, 0xEA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xAB, 0xBB, 0x00, 0x00, 0x00, 0x00,
  0xA5, 0xB5, 0xCF, 0xEF, 0x00, 0x00, 0xC7, 0xE7,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD3, 0xF3,
  0xA2, 0x00, 0x00, 0xA6, 0xB6, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xD1, 0xF1, 0x00,
  0x00, 0x00, 0xBD, 0xBF, 0xD2, 0xF2, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA3, 0xB3,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xA9, 0xB9, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xBC,
  0xDD, 0xFD, 0xDE, 0xFE, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xD9, 0xF9, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xAE, 0xBE, 0x00,
};
const byte_t Latin4Page02[32] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB7,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xFF, 0x00, 0xB2, 0x00, 0x00, 0x00, 0x00,
};

NEVER_INLINE byte_t EncodeExtra(char32_t c) {
  ASSERT(c >= 0xA0);

  if (c < 0x0180)
    return Latin4Page00[c - 0x00A0];
  if (0x02C0 <= c && c < 0x02E0)
    return Latin4Page02[c - 0x02C0];

  return 0;
}

template<typename T>
int EncodeTmpl(Context& context, Span<T> input, MutableBufferSpan output) {
  auto* iptr = begin(input);
  auto* const iptr_end = end(input);
  auto* optr = static_cast<byte_t*>(output.data());
  int oi = 0;
  bool saw_error = false;

  while (iptr < iptr_end) {
    char32_t c = DecodeUtf(iptr, iptr_end);
    if (c < 0xA0) {
      optr[oi++] = static_cast<byte_t>(c);
    } else  {
      byte_t b = EncodeExtra(c);
      if (b != 0) {
        optr[oi++] = b;
      } else {
        optr[oi++] = Unicode::FallbackUnit<char>;
        saw_error = true;
      }
    }
  }
  context.MaybeThrow(saw_error);
  ASSERT(oi <= output.size());
  return oi;
}

int Encode(Context& context, StringSpan input, MutableBufferSpan output) {
  return EncodeTmpl(context, input, output);
}
int Encode16(Context& context, String16Span input, MutableBufferSpan output) {
  return EncodeTmpl(context, input, output);
}

int CountBytes(const Context& context, StringSpan input) {
  return input.size();
}
int CountBytes16(const Context& context, String16Span input) {
  return input.size();
}

static constexpr StringSpan Aliases[] = {
  "iso-ir-110",
  "latin4",
  "L4",
};

constexpr TextCodecVtable Vtable = {
  Decode, Decode16,
  CountChars, CountChars16,
  Encode, Encode16,
  CountBytes, CountBytes16,
};

constexpr auto Build() {
  auto builder = BuildTextCodec("ISO-8859-4", Vtable);
  builder.SetAliases(Aliases);
  builder.SetIanaCodepage(7);
  builder.SetWindowsCodepage(28594);
  return builder;
}

} // namespace

constexpr const TextCodec Latin4Codec = Build();

} // namespace stp