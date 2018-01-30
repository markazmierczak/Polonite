// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncoding.h"

namespace stp {
namespace detail {

namespace {

using Context = TextConversionContext;

// 0x80 - 0x9F
const char16_t Cp1252ToUnicode[32] = {
  0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
  0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x017D, 0x0000,
  0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
  0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x017E, 0x0178,
};

int Decode(Context& context, BufferSpan input, MutableStringSpan output, bool flush) {
  auto* iptr = static_cast<const byte_t*>(input.data());
  const int isize = input.size();
  auto* optr = output.data();
  int oi = 0;
  bool saw_error = false;

  for (int ii = 0; ii < isize; ++ii) {
    byte_t b = iptr[ii];
    if (LIKELY(b < 0x80)) {
      optr[oi++] = static_cast<char>(b);
    } else if (b >= 0xA0) {
      oi += Utf8::EncodeInTwoUnits(optr + oi, b);
    } else {
      char16_t c = Cp1252ToUnicode[b - 0x80];
      if (c) {
        oi += EncodeUtf(optr + oi, c);
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

int Decode16(Context& context, BufferSpan input, MutableString16Span output, bool flush) {
  auto* iptr = static_cast<const byte_t*>(input.data());
  const int isize = input.size();
  auto* optr = output.data();
  int oi = 0;
  bool saw_error = false;

  for (int ii = 0; ii < isize; ++ii) {
    byte_t b = iptr[ii];
    if (LIKELY(b < 0x80 || b >= 0xA0)) {
      optr[oi++] = static_cast<char16_t>(b);
    } else {
      char16_t c = Cp1252ToUnicode[b - 0x80];
      if (c) {
        optr[oi++] = static_cast<char16_t>(c);
      } else {
        optr[oi++] = Unicode::FallbackUnit<char16_t>;
        saw_error = true;
      }
    }
  }
  context.MaybeThrow(saw_error);
  ASSERT(oi <= output.size());
  return oi;
}

template<typename T>
int CountCharsTmpl(const Context& context, BufferSpan input) {
  auto* iptr = static_cast<const byte_t*>(input.data());
  const int isize = input.size();
  int oi = 0;

  for (int i = 0, len = isize; i < len; ++i) {
    byte_t b = iptr[i];
    if (LIKELY(b < 0x80)) {
      oi += 1;
    } else {
      char16_t c = b >= 0xA0 ? static_cast<char16_t>(b) : Cp1252ToUnicode[b - 0x80];
      if (c)
        oi += UtfTmpl<T>::EncodedLength(c);
      else
        oi += 1;
    }
  }
  return oi;
}

int CountChars(const Context& context, BufferSpan input) {
  return CountCharsTmpl<char>(context, input);
}
int CountChars16(const Context& context, BufferSpan input) {
  return CountCharsTmpl<char16_t>(context, input);
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

NEVER_INLINE byte_t EncodeExtra(char32_t c) {
  ASSERT(!stp::IsAscii(c));
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
int EncodeTmpl(Context& context, Span<T> input, MutableBufferSpan output) {
  auto* iptr = begin(input);
  auto* const iptr_end = end(input);
  int oi = 0;
  auto* optr = static_cast<byte_t*>(output.data());
  bool saw_error = false;

  while (iptr < iptr_end) {
    char32_t c = DecodeUtf(iptr, iptr_end);
    if ((c < 0x80) || (0x00A0 <= c && c < 0x0100)) {
      optr[oi++] = static_cast<byte_t>(c);
    } else {
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

constexpr StringSpan Aliases[] = {
  "windows-1252",
};

constexpr TextCodecVtable Vtable = {
  Decode, Decode16,
  CountChars, CountChars16,
  Encode, Encode16,
  CountBytes, CountBytes16,
};

constexpr auto Build() {
  auto builder = BuildTextCodec("cp1252", Vtable);
  builder.SetAliases(Aliases);
  builder.SetIanaCodepage(2252);
  builder.SetWindowsCodepage(1252);
  return builder;
}

} // namespace

constexpr const TextCodec Cp1252Codec = Build();

} // namespace detail
} // namespace stp
