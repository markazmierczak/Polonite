// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Codec/TextCodec.h"

#include "Base/Compiler/ByteOrder.h"

namespace stp {

namespace {

using Context = TextConversionContext;

template<ByteOrder TOrder>
inline char32_t DecodeOne(const byte_t* b) {
  if (TOrder == ByteOrder::LittleEndian)
    return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
  else
    return b[3] | (b[2] << 8) | (b[1] << 16) | (b[0] << 24);
}

class Utf32ReaderState {
 public:
  explicit Utf32ReaderState(const byte_t* bytes) : bytes_(const_cast<byte_t*>(bytes)) {}

  bool MaybeFeed(const byte_t* iptr, int& ii, const int isize) {
    if (!NeedsFlush())
      return false;
    byte_t& len = bytes_[4];
    while (len < 4 && ii < isize)
      bytes_[len++] = iptr[ii++];
    return len == 4;
  }

  bool Feed(const byte_t* iptr, int& ii, const int isize) {
    ASSERT(!NeedsFlush());
    ASSERT(ii < isize && isize - ii < 4);
    byte_t n = static_cast<byte_t>(isize - ii);
    memcpy(bytes_, iptr, n);
    bytes_[4] = n;
    return false;
  }

  template<ByteOrder TOrder, typename TChar>
  void Write(TChar* optr, int& oi, bool& saw_error) {
    ASSERT(NeedsFlush());
    char32_t c = DecodeOne<TOrder>(bytes_);
    if (GetLength() == 4 && Unicode::IsValidCodepoint(c)) {
      oi += EncodeUtf(optr, c);
    } else {
      optr[oi++] = Unicode::FallbackUnit<TChar>;
      saw_error = true;
    }
    bytes_[4] = 0;
  }

  bool NeedsFlush() const { return GetLength() != 0; }
  int CountChars() const { return NeedsFlush() ? 4 : 0; }

  int GetLength() const { return bytes_[4]; }

 private:
  byte_t* const bytes_;
};

template<ByteOrder TOrder, typename T>
int DecodeTmpl(Context& context, BufferSpan input, MutableSpan<T> output, bool flush) {
  Utf32ReaderState state(context.state.bytes);

  const int isize = input.size();
  auto* iptr = static_cast<const byte_t*>(input.data());
  int ii = 0;
  auto* optr = output.data();
  int oi = 0;
  bool saw_error = false;

  if (state.MaybeFeed(iptr, ii, isize))
    state.Write<TOrder>(optr, oi, saw_error);

  for (int iend = isize - 4; ii <= iend; ii += 4) {
    char32_t c = DecodeOne<TOrder>(iptr + ii);
    if (Unicode::IsValidCodepoint(c)) {
      oi += EncodeUtf(optr + oi, c);
    } else {
      optr[oi++] = Unicode::FallbackUnit<T>;
      saw_error = true;
    }
  }

  if (ii < isize)
    state.Feed(iptr, ii, isize);
  if (flush && state.NeedsFlush())
    state.Write<TOrder>(optr, oi, saw_error);

  context.MaybeThrow(saw_error);
  ASSERT(oi <= output.size());
  return oi;
}

int DecodeLE(Context& context, BufferSpan input, MutableStringSpan output, bool flush) {
  return DecodeTmpl<ByteOrder::LittleEndian>(context, input, output, flush);
}
int Decode16LE(Context& context, BufferSpan input, MutableString16Span output, bool flush) {
  return DecodeTmpl<ByteOrder::LittleEndian>(context, input, output, flush);
}
int DecodeBE(Context& context, BufferSpan input, MutableStringSpan output, bool flush) {
  return DecodeTmpl<ByteOrder::BigEndian>(context, input, output, flush);
}
int Decode16BE(Context& context, BufferSpan input, MutableString16Span output, bool flush) {
  return DecodeTmpl<ByteOrder::BigEndian>(context, input, output, flush);
}

template<ByteOrder TOrder>
int CountCharsTmpl(const Context& context, BufferSpan input) {
  Utf32ReaderState state(context.state.bytes);

  const int isize = input.size();
  auto* iptr = static_cast<const byte_t*>(input.data());
  int ii = 0;
  int oi = 0;

  if (state.NeedsFlush()) {
    ii += 4 - state.GetLength();
    oi += 4;
  }

  for (int iend = isize - 4; ii <= iend; ii += 4) {
    char32_t c = DecodeOne<TOrder>(iptr + ii);
    if (c < 0x80)
      oi += 1;
    else if (c < 0x800)
      oi += 2;
    else
      oi += 4;
  }
  if (ii < isize)
    oi += 4;
  return oi;
}

int CountCharsLE(const Context& context, BufferSpan input) {
  return CountCharsTmpl<ByteOrder::LittleEndian>(context, input);
}
int CountCharsBE(const Context& context, BufferSpan input) {
  return CountCharsTmpl<ByteOrder::BigEndian>(context, input);
}

template<ByteOrder TOrder>
int CountChars16Tmpl(const Context& context, BufferSpan input) {
  Utf32ReaderState state(context.state.bytes);

  const int isize = input.size();
  auto* iptr = static_cast<const byte_t*>(input.data());
  int ii = 0;
  int oi = 0;

  if (state.NeedsFlush()) {
    ii += 4 - state.GetLength();
    oi += 2;
  }

  for (int iend = isize - 4; ii <= iend; ii += 4) {
    char32_t c = DecodeOne<TOrder>(iptr + ii);
    if (c < Unicode::MinLeadSurrogate)
      oi += 1;
    else
      oi += 2;
  }
  if (ii < isize)
    oi += 2;
  return oi;
}

int CountChars16LE(const Context& context, BufferSpan input) {
  return CountChars16Tmpl<ByteOrder::LittleEndian>(context, input);
}
int CountChars16BE(const Context& context, BufferSpan input) {
  return CountChars16Tmpl<ByteOrder::BigEndian>(context, input);
}

template<ByteOrder TOrder>
int EncodeOne(char32_t c, byte_t* out) {
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
int EncodeTmpl(Context& context, Span<T> input, MutableBufferSpan output) {
  auto* iptr = begin(input);
  auto* const iptr_end = end(input);
  auto* optr = static_cast<byte_t*>(output.data());
  int oi = 0;
  bool saw_error = false;

  while (iptr < iptr_end) {
    char32_t c = DecodeUtf(iptr, iptr_end);
    if (!Unicode::IsDecodeError(c)) {
      oi += EncodeOne<TOrder>(c, optr + oi);
    } else {
      oi += EncodeOne<TOrder>(Unicode::FallbackUnit<char32_t>, optr + oi);
      saw_error = true;
    }
  }
  context.MaybeThrow(saw_error);
  ASSERT(oi <= output.size());
  return oi;
}

int EncodeLE(Context& context, StringSpan input, MutableBufferSpan output) {
  return EncodeTmpl<ByteOrder::LittleEndian>(context, input, output);
}
int Encode16LE(Context& context, String16Span input, MutableBufferSpan output) {
  return EncodeTmpl<ByteOrder::LittleEndian>(context, input, output);
}
int EncodeBE(Context& context, StringSpan input, MutableBufferSpan output) {
  return EncodeTmpl<ByteOrder::BigEndian>(context, input, output);
}
int Encode16BE(Context& context, String16Span input, MutableBufferSpan output) {
  return EncodeTmpl<ByteOrder::BigEndian>(context, input, output);
}

int CountBytes(const Context& context, StringSpan input) {
  return input.size() << 2;
}
int CountBytes16(const Context& context, String16Span input) {
  return input.size() << 2;
}

constexpr TextCodecVtable VtableLE = {
  DecodeLE, Decode16LE,
  CountCharsLE, CountChars16LE,
  EncodeLE, Encode16LE,
  CountBytes, CountBytes16,
};

constexpr TextCodecVtable VtableBE = {
  DecodeBE, Decode16BE,
  CountCharsBE, CountChars16BE,
  EncodeBE, Encode16BE,
  CountBytes, CountBytes16,
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

constexpr const TextCodec Utf32LECodec = BuildLE();
constexpr const TextCodec Utf32BECodec = BuildBE();
constexpr const TextCodec Utf32Codec = Build();

} // namespace stp
