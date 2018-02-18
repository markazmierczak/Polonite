// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Codec/AsciiEncoding.h"

#include "Base/Text/TextEncodingDataBuilder.h"
#include "Base/Text/Utf.h"

namespace stp {

TextDecoder::Result AsciiDecoder::Decode(BufferSpan input, MutableStringSpan output, bool flush) {
  auto* input_data = static_cast<const byte_t*>(input.data());
  int num_read = 0;
  int num_wrote = 0;
  bool more_output = false;

  while (num_read < input.size()) {
    if (!(num_wrote < output.size())) {
      more_output = true;
      break;
    }
    byte_t b = input_data[num_read];
    if (LIKELY(b < 0x80)) {
      output[num_wrote++] = static_cast<char>(b);
    } else {
      int num_encoded = TryEncodeUtf(unicode::ReplacementRune, output.getSlice(num_wrote));
      if (num_encoded == 0) {
        more_output = true;
        break;
      }
      num_wrote += num_encoded;
    }
    ++num_read;
  }
  return Result(num_read, num_wrote, more_output);
}

TextEncoder::Result AsciiEncoder::Encode(StringSpan input, MutableBufferSpan output) {
  auto* iptr = begin(input);
  auto* const iptr_end = end(input);
  auto* output_data = static_cast<byte_t*>(output.data());
  int num_wrote = 0;
  bool more_output = false;

  while (iptr < iptr_end) {
    if (!(num_wrote < output.size())) {
      more_output = true;
      break;
    }
    char32_t c = TryDecodeUtf(iptr, iptr_end);
    if (UNLIKELY(!isAscii(c))) {
      c = '?';
    }
    output_data[num_wrote++] = static_cast<byte_t>(c);
  }
  return Result(iptr - input.data(), num_wrote, more_output);
}

namespace detail {

constexpr const TextEncodingData AsciiEncodingData =
    BuildTextEncodingData<AsciiDecoder, AsciiEncoder>("US-ASCII");

} // namespace detail

} // namespace stp
