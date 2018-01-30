// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncoding.h"

namespace stp {
namespace detail {

namespace {

using Context = TextConversionContext;

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
      oi += Utf8::EncodeInTwoUnits(optr + oi, b);
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
    optr[oi++] = static_cast<char16_t>(iptr[ii]);
  }
  ASSERT(oi < output.size());
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

template<typename T>
int EncodeTmpl(Context& context, Span<T> input, MutableBufferSpan output) {
  auto* iptr = begin(input);
  auto* const iptr_end = end(input);
  auto* optr = static_cast<byte_t*>(output.data());
  int oi = 0;
  bool saw_error = false;

  while (iptr < iptr_end) {
    char32_t c = DecodeUtf(iptr, iptr_end);
    if (LIKELY(c < 0x100)) {
      optr[oi++] = static_cast<byte_t>(c);
    } else {
      optr[oi++] = Unicode::FallbackUnit<char>;
      saw_error = true;
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
  "iso-ir-100",
  "latin1",
  "L1",
  "IBM819",
  "CP819",
};

constexpr TextCodecVtable Vtable = {
  Decode, Decode16,
  CountChars, CountChars16,
  Encode, Encode16,
  CountBytes, CountBytes16,
};

constexpr auto Build() {
  auto builder = BuildTextCodec("ISO-8859-1", Vtable);
  builder.SetAliases(Aliases);
  builder.SetIanaCodepage(4);
  builder.SetWindowsCodepage(28591);
  return builder;
}

} // namespace

constexpr const TextCodec Latin1Codec = Build();

} // namespace detail
} // namespace stp
