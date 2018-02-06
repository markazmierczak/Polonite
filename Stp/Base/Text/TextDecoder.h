// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_TEXTDECODER_H_
#define STP_BASE_TEXT_TEXTDECODER_H_

#include "Base/Text/TextEncoding.h"

namespace stp {

class BASE_EXPORT TextDecoder {
 public:
  explicit TextDecoder(TextEncoding codec);

  TextConversionResult Convert(BufferSpan input, MutableStringSpan output, bool flush);
  TextConversionResult Convert(BufferSpan input, MutableString16Span output, bool flush);

 private:
  const TextCodecVtable& vtable_;
  TextConversionContext* context_ = nullptr;
};

inline TextDecoder::TextDecoder(TextEncoding codec)
    : vtable_(codec.GetVtable()) {
}

inline TextConversionResult TextDecoder::Convert(
    BufferSpan input, MutableStringSpan output, bool flush) {
  return vtable_.decode(context_, input, output, flush);
}
inline TextConversionResult TextDecoder::Convert(
    BufferSpan input, MutableString16Span output, bool flush) {
  return vtable_.decode16(context_, input, output, flush);
}

} // namespace stp

#endif // STP_BASE_TEXT_TEXTDECODER_H_
