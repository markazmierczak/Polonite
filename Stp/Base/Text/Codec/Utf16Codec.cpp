// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncoding.h"

#include "Base/Compiler/ByteOrder.h"

// TODO Faster encode/decode between char16_t (only validation)

namespace stp {
namespace detail {

using Context = TextConversionContext;

namespace {

template<ByteOrder TOrder>
inline char16_t DecodeOne(const byte_t* b) {
  if (TOrder == ByteOrder::LittleEndian)
    return b[0] | (b[1] << 8);
  else
    return b[1] | (b[0] << 8);
}

template<ByteOrder TOrder>
class Utf16ReaderState {
 public:
  explicit Utf16ReaderState(const byte_t* bytes) : bytes_(const_cast<byte_t*>(bytes)) {}

  bool MaybeFeed(const byte_t* iptr, int& ii, const int isize) {
    if (!NeedsFlush())
      return false;
    byte_t& len = bytes_[4];
    char16_t lead;
    if (len == 1) {
      if (ii == isize)
        return false;
      bytes_[len++] = iptr[ii++];
      lead = DecodeOne<TOrder>(bytes_);
      if (!Unicode::IsLeadSurrogate(lead))
        return true;
    }
    while (len < 4 && ii < isize)
      bytes_[len++] = iptr[ii++];
    return len == 4;
  }

  bool Feed(const byte_t* iptr, int& ii, const int isize) {
    ASSERT(!NeedsFlush());
    ASSERT(ii < isize);
    byte_t& len = bytes_[4];
    bytes_[len++] = iptr[ii++];
    if (ii == isize)
      return false;
    bytes_[len++] = iptr[ii++];
    char16_t lead = DecodeOne<TOrder>(bytes_);
    if (!Utf16::IsLeadSurrogate(lead))
      return true;
    bytes_[len++] = iptr[ii++];
    if (ii == isize)
      return false;
    bytes_[len++] = iptr[ii++];
    return true;
  }

  template<typename TChar>
  void Write(TChar* optr, int& oi, bool& saw_error) {
    ASSERT(NeedsFlush());
    bool error = GetLength() == 1 || GetLength() == 3;

    char16_t lead = DecodeOne<TOrder>(bytes_);
    if (!error) {
      if (GetLength() == 2) {
        error = Utf16::IsSurrogate(lead);
        if (!error)
          oi += EncodeUtf(optr + oi, lead);
      } else {
        char16_t trail = DecodeOne<TOrder>(bytes_ + 2);
        error = !Utf16::IsTrailSurrogate(trail);
        if (!error) {
          char32_t lc = Utf16::DecodeSurrogatePair(lead, trail);
          oi += EncodeUtf(optr + oi, lc);
        }
      }
    }
    if (error) {
      optr[oi++] = Unicode::FallbackUnit<TChar>;
      saw_error = true;
    }
    bytes_[4] = 0;
  }

  bool NeedsFlush() const { return GetLength() != 0; }
  int CountChars() const { return NeedsFlush() ? 4 : 0; }
  int CountChars16() const { return NeedsFlush() ? 2 : 0; }

  int GetLength() const { return bytes_[4]; }

 private:
  byte_t* const bytes_;
};

template<ByteOrder TOrder, typename T>
int DecodeTmpl(Context& context, BufferSpan input, MutableSpan<T> output, bool flush) {
  Utf16ReaderState<TOrder> state(context.state.bytes);

  const int isize = input.size();
  auto* iptr = static_cast<const byte_t*>(input.data());
  int ii = 0;
  auto* optr = output.data();
  int oi = 0;
  bool saw_error = false;

  if (state.MaybeFeed(iptr, ii, isize))
    state.Write(optr, oi, saw_error);

  const int isize_fast = isize - 4;
  while (ii <= isize_fast) {
    char16_t lead = DecodeOne<TOrder>(iptr + ii);
    ii += 2;
    if (!Utf16::IsSurrogate(lead)) {
      oi += EncodeUtf(optr + oi, lead);
    } else {
      bool error = false;
      if (Utf16::IsLeadSurrogate(lead)) {
        char16_t trail = DecodeOne<TOrder>(iptr + ii);
        ii += 2;
        if (Utf16::IsTrailSurrogate(lead)) {
          char32_t lc = Utf16::DecodeSurrogatePair(lead, trail);
          oi += EncodeUtf(optr + oi, lc);
        } else {
          error = true;
        }
      } else {
        error = true;
      }
      if (error) {
        optr[oi++] = Unicode::FallbackUnit<T>;
        saw_error = true;
      }
    }
  }

  while (ii < isize) {
    if (state.Feed(iptr, ii, isize))
      state.Write(optr, oi, saw_error);
  }
  if (flush && state.NeedsFlush())
    state.Write(optr, oi, saw_error);

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
  Utf16ReaderState<TOrder> state(context.state.bytes);

  const int isize = input.size();
  auto* iptr = static_cast<const byte_t*>(input.data());
  int ii = 0;
  int oi = 0;

  if (state.NeedsFlush()) {
    if (state.GetLength() & 1)
      ++ii;
    oi += 4;
  }

  for (int iend = isize - 2; ii <= iend; ii += 2) {
    char16_t c = DecodeOne<TOrder>(iptr + ii);
    if (c < 0x80)
      oi += 1;
    else if (c < 0x800)
      oi += 2;
    else
      oi += 3;
  }
  if (ii < isize)
    oi += 1;
  return oi;
}

template<ByteOrder TOrder>
int CountChars16Tmpl(const Context& context, BufferSpan input) {
  Utf16ReaderState<TOrder> state(context.state.bytes);
  return ((input.size() + 1) >> 1) + state.CountChars16();
}

int CountCharsLE(const Context& context, BufferSpan input) {
  return CountCharsTmpl<ByteOrder::LittleEndian>(context, input);
}
int CountCharsBE(const Context& context, BufferSpan input) {
  return CountCharsTmpl<ByteOrder::BigEndian>(context, input);
}
int CountChars16LE(const Context& context, BufferSpan input) {
  return CountChars16Tmpl<ByteOrder::LittleEndian>(context, input);
}
int CountChars16BE(const Context& context, BufferSpan input) {
  return CountChars16Tmpl<ByteOrder::BigEndian>(context, input);
}

template<ByteOrder TOrder>
int EncodeOne(char16_t c, byte_t* out) {
  if (TOrder == ByteOrder::LittleEndian) {
    out[0] = static_cast<byte_t>((c >> 0) & 0xFF);
    out[1] = static_cast<byte_t>((c >> 8) & 0xFF);
  } else {
    out[1] = static_cast<byte_t>((c >> 0) & 0xFF);
    out[0] = static_cast<byte_t>((c >> 8) & 0xFF);
  }
  return 4;
}

template<ByteOrder TOrder, typename T>
int EncodeTmpl(Context& context, Span<T> input, MutableBufferSpan output) {
  auto* inp = begin(input);
  auto* const inp_end = end(input);
  auto* outp = static_cast<byte_t*>(output.data());
  int oi = 0;
  bool saw_error = false;

  while (inp < inp_end) {
    char32_t c = DecodeUtf(inp, inp_end);
    if (!Unicode::IsDecodeError(c)) {
      oi += EncodeOne<TOrder>(c, outp + oi);
    } else {
      oi += EncodeOne<TOrder>(Unicode::FallbackUnit<char16_t>, outp + oi);
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
  return input.size() << 1;
}
int CountBytes16(const Context& context, String16Span input) {
  return input.size();
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
  auto builder = BuildTextCodec("UTF-16LE", VtableLE);
  builder.SetIanaCodepage(1014);
  builder.SetWindowsCodepage(1200);
  return builder;
}

constexpr auto BuildBE() {
  auto builder = BuildTextCodec("UTF-16BE", VtableBE);
  builder.SetIanaCodepage(1013);
  builder.SetWindowsCodepage(1201);
  return builder;
}

constexpr auto Build() {
  auto builder = BuildTextCodec("UTF-16", VtableBE);
  builder.SetIanaCodepage(1015);
  return builder;
}

} // namespace

constexpr const TextCodec Utf16LECodec = BuildLE();
constexpr const TextCodec Utf16BECodec = BuildBE();
constexpr const TextCodec Utf16Codec = Build();

} // namespace detail
} // namespace stp
