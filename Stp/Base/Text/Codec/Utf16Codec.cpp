// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncoding.h"

#include "Base/Compiler/Endianness.h"
#include "Base/Text/Utf.h"

// TODO Faster encode/decode between char16_t (only validation)

namespace stp {

namespace {

template<Endianness TOrder>
inline char16_t DecodeOne(const byte_t* b) {
  if (TOrder == Endianness::Little)
    return b[0] | (b[1] << 8);
  else
    return b[1] | (b[0] << 8);
}

template<Endianness TOrder>
inline TextConversionResult Decode(
    TextConversionContext* context, BufferSpan input, MutableStringSpan output, bool flush) {
  auto* input_data = static_cast<const byte_t*>(input.data());
  int num_read = 0;
  auto* output_data = output.data();
  int max_output_size = output.size() - Utf8::MaxEncodedRuneLength;
  int num_wrote = 0;
  bool did_fallback = false;

  const int fast_input_size = input.size() & ~1;

  while (num_read <= fast_input_size && num_wrote < max_output_size) {
    char16_t lead = DecodeOne<TOrder>(input_data + num_read);
    num_read += 2;
    if (!unicode::IsSurrogate(lead)) {
      num_wrote += EncodeUtf(output_data + num_wrote, lead);
    } else {
      bool error = false;
      if (unicode::IsLeadSurrogate(lead)) {
        char16_t trail = DecodeOne<TOrder>(input_data + num_read);
        num_read += 2;
        if (unicode::IsTrailSurrogate(lead)) {
          char32_t lc = unicode::DecodeSurrogatePair(lead, trail);
          num_wrote += EncodeUtf(output_data + num_wrote, lc);
        } else {
          error = true;
        }
      } else {
        error = true;
      }
      if (error) {
        output_data[num_wrote++] = Unicode::FallbackUnit<T>;
        did_fallback = true;
      }
    }
  }

  while (num_read < isize) {
    if (state.Feed(input_data, num_read, isize))
      state.Write(output_data, num_wrote, did_fallback);
  }
  if (flush && state.NeedsFlush())
    state.Write(output_data, num_wrote, did_fallback);

  return TextConversionResult(num_read, num_wrote, false);
}

TextConversionResult DecodeLE(
    TextConversionContext* context, BufferSpan input, MutableStringSpan output, bool flush) {
  return DecodeTmpl<Endianness::Little>(context, input, output, flush);
}
TextConversionResult DecodeBE(
    TextConversionContext* context, BufferSpan input, MutableStringSpan output, bool flush) {
  return DecodeTmpl<Endianness::Big>(context, input, output, flush);
}

template<Endianness TOrder>
inline void EncodeOne(char16_t c, byte_t* out) {
  if (TOrder == Endianness::Little) {
    out[0] = static_cast<byte_t>((c >> 0) & 0xFF);
    out[1] = static_cast<byte_t>((c >> 8) & 0xFF);
  } else {
    out[1] = static_cast<byte_t>((c >> 0) & 0xFF);
    out[0] = static_cast<byte_t>((c >> 8) & 0xFF);
  }
}

template<Endianness TOrder, typename T>
TextConversionResult EncodeTmpl(
    TextConversionContext* context, Span<T> input, MutableBufferSpan output) {
  auto* iptr = begin(input);
  auto* const iptr_end = end(input);

  auto* output_data = static_cast<byte_t*>(output.data());
  int num_wrote = 0;
  int max_output_size = output.size() - Utf16::MaxEncodedRuneLength * isizeof(char16_t);

  bool did_fallback = false;

  while (iptr < iptr_end && num_wrote < max_output_size) {
    char32_t c = TryDecodeUtf(iptr, iptr_end);
    if (unicode::IsDecodeError(c)) {
      c = unicode::ReplacementRune;
      did_fallback = true;
    }
    char16_t h[2];
    int num_encoded = Utf16::Encode(h, c);
    EncodeOne<TOrder>(c, output_data + num_wrote);
    if (num_encoded == 2) {
      EncodeOne<TOrder>(c, output_data + num_wrote + 2);
    }
    num_wrote += num_encoded << 1;
  }
  return TextConversionResult(iptr - input.data(), num_wrote, did_fallback);
}

TextConversionResult EncodeLE(
    TextConversionContext* context, StringSpan input, MutableBufferSpan output) {
  return EncodeTmpl<Endianness::Little>(context, input, output);
}
TextConversionResult EncodeBE(
    TextConversionContext* context, StringSpan input, MutableBufferSpan output) {
  return EncodeTmpl<Endianness::Big>(context, input, output);
}

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

namespace detail {
constexpr const TextCodec Utf16LECodec = BuildLE();
constexpr const TextCodec Utf16BECodec = BuildBE();
constexpr const TextCodec Utf16Codec = Build();
} // namespace detail

} // namespace stp
