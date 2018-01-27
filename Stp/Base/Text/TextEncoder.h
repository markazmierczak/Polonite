// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_TEXTENCODER_H_
#define STP_BASE_TEXT_TEXTENCODER_H_

#include "Base/Text/TextEncoding.h"

namespace stp {

class BASE_EXPORT TextEncoder {
 public:
  explicit TextEncoder(TextEncoding codec);

  void SetExceptionFallback() { context_.exception_on_fallback = true; }

  int Convert(StringSpan input, MutableBufferSpan output);
  int Convert(String16Span input, MutableBufferSpan output);
  int CountBytes(StringSpan input) const;
  int CountBytes(String16Span input) const;

 private:
  const TextCodecVtable& vtable_;
  TextConversionContext context_;
};

inline TextEncoder::TextEncoder(TextEncoding codec)
    : vtable_(codec.GetVtable()) {
}

inline int TextEncoder::Convert(StringSpan input, MutableBufferSpan output) {
  return vtable_.encode(context_, input, output);
}
inline int TextEncoder::Convert(String16Span input, MutableBufferSpan output) {
  return vtable_.encode16(context_, input, output);
}

inline int TextEncoder::CountBytes(StringSpan input) const {
  return vtable_.count_bytes(context_, input);
}
inline int TextEncoder::CountBytes(String16Span input) const {
  return vtable_.count_bytes16(context_, input);
}

} // namespace stp

#endif // STP_BASE_TEXT_TEXTENCODER_H_
