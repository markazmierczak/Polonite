// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_CODEC_UTF8ENCODING_H_
#define STP_BASE_TEXT_CODEC_UTF8ENCODING_H_

#include "Base/Text/TextEncoding.h"

namespace stp {

namespace detail {
BASE_EXPORT extern const TextEncodingData Utf8EncodingData;
}

namespace BuiltinTextEncodings {
inline TextEncoding Utf8() { return TextEncoding(&detail::Utf8EncodingData); }
}

class Utf8Decoder : public TextDecoder {
 public:
  Result Decode(BufferSpan input, MutableStringSpan output, bool flush) override;
};

class Utf8Encoder : public TextEncoder {
 public:
  Result Encode(StringSpan input, MutableBufferSpan output) override;
};

} // namespace stp

#endif // STP_BASE_TEXT_CODEC_UTF8ENCODING_H_
