// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_CODEC_CP1252ENCODING_H_
#define STP_BASE_TEXT_CODEC_CP1252ENCODING_H_

#include "Base/Text/TextEncoding.h"

namespace stp {

namespace detail {
BASE_EXPORT extern const TextEncodingData Cp1252EncodingData;
}

namespace BuiltinTextEncodings {
inline TextEncoding Cp1252() { return TextEncoding(&detail::Cp1252EncodingData); }
}

class Cp1252Decoder : public TextDecoder {
 public:
  Result Decode(BufferSpan input, MutableStringSpan output, bool flush) override;
};

class Cp1252Encoder : public TextEncoder {
 public:
  Result Encode(StringSpan input, MutableBufferSpan output) override;
};

} // namespace stp

#endif // STP_BASE_TEXT_CODEC_CP1252ENCODING_H_
