// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_CODEC_ASCIIENCODING_H_
#define STP_BASE_TEXT_CODEC_ASCIIENCODING_H_

#include "Base/Text/TextEncoding.h"

namespace stp {

namespace detail {
BASE_EXPORT extern const TextEncodingData AsciiEncodingData;
}

namespace BuiltinTextEncodings {
inline TextEncoding Ascii() { return TextEncoding(&detail::AsciiEncodingData); }
}

class AsciiDecoder : public TextDecoder {
 public:
  explicit AsciiDecoder(const TextEncodingConfig* config) {}
  Result Decode(BufferSpan input, MutableStringSpan output, bool flush) override;
};

class AsciiEncoder : public TextEncoder {
 public:
  explicit AsciiEncoder(const TextEncodingConfig* config) {}
  Result Encode(StringSpan input, MutableBufferSpan output) override;
};

} // namespace stp

#endif // STP_BASE_TEXT_CODEC_ASCIIENCODING_H_
