// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_CODEC_UTF32ENCODING_H_
#define STP_BASE_TEXT_CODEC_UTF32ENCODING_H_

#include "Base/Compiler/Endianness.h"
#include "Base/Text/TextEncoding.h"

namespace stp {

namespace detail {
BASE_EXPORT extern const TextEncodingData Utf32EncodingData;
BASE_EXPORT extern const TextEncodingData Utf32EncodingDataBE;
BASE_EXPORT extern const TextEncodingData Utf32EncodingDataLE;
}

namespace BuiltinTextEncodings {
inline TextEncoding Utf32() { return TextEncoding(&detail::Utf32EncodingData); }
inline TextEncoding Utf32BE() { return TextEncoding(&detail::Utf32EncodingDataBE); }
inline TextEncoding Utf32LE() { return TextEncoding(&detail::Utf32EncodingDataLE); }
}

struct Utf32EncodingConfig : public TextEncodingConfig {
  Endianness endianness = Endianness::Big;
  bool writes_bom = false;
  bool accepts_bom = false;
  bool requires_bom = false;
};

class Utf32Decoder : public TextDecoder {
 public:
  explicit Utf32Decoder(const TextEncodingConfig* config)
      : config_(static_cast<const Utf32EncodingConfig&>(*config)) {}

  Result Decode(BufferSpan input, MutableStringSpan output, bool flush) override;

 private:
  const Utf32EncodingConfig& config_;
};

class Utf32Encoder : public TextEncoder {
 public:
  explicit Utf32Encoder(const TextEncodingConfig* config)
      : config_(static_cast<const Utf32EncodingConfig&>(*config)) {}

  Result Encode(StringSpan input, MutableBufferSpan output) override;

 private:
  const Utf32EncodingConfig& config_;
  bool initial_ = true;
};

} // namespace stp

#endif // STP_BASE_TEXT_CODEC_UTF32ENCODING_H_
