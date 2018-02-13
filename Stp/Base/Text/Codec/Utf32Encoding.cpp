// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Codec/Utf32Encoding.h"

#include "Base/Compiler/Endianness.h"
#include "Base/Text/TextEncodingDataBuilder.h"
#include "Base/Text/Utf.h"

namespace stp {

static char32_t DecodeUtf32Unit(Endianness endianness, const byte_t* b) {
  if (endianness == Endianness::Little)
    return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
  else
    return b[3] | (b[2] << 8) | (b[1] << 16) | (b[0] << 24);
}

TextDecoder::Result Utf32Decoder::Decode(
    BufferSpan input, MutableStringSpan output, bool flush) {
  auto* input_data = static_cast<const byte_t*>(input.data());
  int num_read = 0;

  // Align the number of input bytes to UTF-32 unit size.
  int input_size = input.size();
  int leftover = input_size & 3;
  input_size &= ~3;

  int num_wrote = 0;

  bool more_output = false;
  auto endianness = config_.endianness;

  while (num_read < input_size) {
    if (!(num_wrote < output.size())) {
      more_output = true;
      break;
    }
    char32_t c = DecodeUtf32Unit(endianness, input_data + num_read);
    if (!unicode::IsValidRune(c)) {
      c = unicode::ReplacementRune;
    }
    int encoded_size = TryEncodeUtf(c, output.GetSlice(num_wrote));
    if (encoded_size == 0)
      break;
    num_wrote += encoded_size;
  }
  if (flush && leftover != 0) {
    int encoded = TryEncodeUtf(unicode::ReplacementRune, output.GetSlice(num_wrote));
    if (encoded != 0) {
      more_output = true;
      num_read += leftover;
      num_wrote += encoded;
    }
  }
  return Result(num_read, num_wrote, more_output);
}

static int EncodeUtf32Unit(Endianness endianness, char32_t c, byte_t* out) {
  if (endianness == Endianness::Little) {
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

TextEncoder::Result Utf32Encoder::Encode(StringSpan input, MutableBufferSpan output) {
  auto* iptr = begin(input);
  auto* const iptr_end = end(input);
  auto* output_data = static_cast<byte_t*>(output.data());
  int num_wrote = 0;
  bool more_output = false;
  auto endianness = config_.endianness;

  // Align the output size to UTF-32 unit size.
  int max_output = output.size() & ~3;
  while (iptr < iptr_end) {
    if (!(num_wrote < max_output)) {
      more_output = true;
      break;
    }
    char32_t c = DecodeUtf(iptr, iptr_end);
    if (UtfBase::IsDecodeError(c)) {
      c = unicode::ReplacementRune;
    }
    num_wrote += EncodeUtf32Unit(endianness, c, output_data + num_wrote);
  }
  return Result(iptr - input.data(), num_wrote, more_output);
}

static constexpr Utf32EncodingConfig BuildUtf32Config(Endianness endianness, bool ignore_bom) {
  Utf32EncodingConfig config;
  config.endianness = endianness;
  config.writes_bom = !ignore_bom;
  config.accepts_bom = !ignore_bom;
  return config;
}

static constexpr Utf32EncodingConfig Utf32Config =
    BuildUtf32Config(Endianness::Big, false);
static constexpr Utf32EncodingConfig Utf32BEConfig =
    BuildUtf32Config(Endianness::Big, true);
static constexpr Utf32EncodingConfig Utf32LEConfig =
    BuildUtf32Config(Endianness::Little, true);

constexpr auto Build(StringSpan name, const Utf32EncodingConfig* config) {
  auto builder = BuildTextEncodingData<Utf32Decoder, Utf32Encoder>(name);
  builder.SetConfig(&Utf32Config);
  return builder;
}

namespace detail {
constexpr const TextEncodingData Utf32EncodingData = Build("UTF-32", &Utf32Config);
constexpr const TextEncodingData Utf32BEEncodingData = Build("UTF-32BE", &Utf32BEConfig);
constexpr const TextEncodingData Utf32LEEncodingData = Build("UTF-32LE", &Utf32LEConfig);
} // namespace detail

} // namespace stp
