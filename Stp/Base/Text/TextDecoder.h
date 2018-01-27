// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_TEXTDECODER_H_
#define STP_BASE_TEXT_TEXTDECODER_H_

#include "Base/Text/TextEncoding.h"

namespace stp {

class BASE_EXPORT TextDecoder {
 public:
  explicit TextDecoder(TextEncoding codec);

  void SetExceptionFallback() { context_.exception_on_fallback = true; }

  int Convert(BufferSpan input, MutableStringSpan output, bool flush);
  int Convert(BufferSpan input, MutableString16Span output, bool flush);

  template<typename T>
  int CountChars(BufferSpan input) const { return CountCharsImpl(input, T()); }

 private:
  int CountCharsImpl(BufferSpan input, char) const;
  int CountCharsImpl(BufferSpan input, char16_t) const;

  const TextCodecVtable& vtable_;
  TextConversionContext context_;
};

inline TextDecoder::TextDecoder(TextEncoding codec)
    : vtable_(codec.GetVtable()) {
}

inline int TextDecoder::Convert(BufferSpan input, MutableStringSpan output, bool flush) {
  return vtable_.decode(context_, input, output, flush);
}
inline int TextDecoder::Convert(BufferSpan input, MutableString16Span output, bool flush) {
  return vtable_.decode16(context_, input, output, flush);
}

inline int TextDecoder::CountCharsImpl(BufferSpan input, char) const {
  return vtable_.count_chars(context_, input);
}
inline int TextDecoder::CountCharsImpl(BufferSpan input, char16_t) const {
  return vtable_.count_chars16(context_, input);
}

template<typename TOutput, TEnableIf<TIsStringContainer<TOutput>>* = nullptr>
void AppendEncoded(TOutput& output, BufferSpan text, TextEncoding encoding) {
  using CharType = typename TOutput::ItemType;
  TextDecoder decoder(encoding);
  int n = decoder.CountChars<CharType>(text);
  auto* dst = output.AppendUninitialized(n);
  int rn = decoder.Convert(text, MutableSpan<CharType>(dst, n), true);
  output.RemoveSuffix(n - rn);
}

} // namespace stp

#endif // STP_BASE_TEXT_TEXTDECODER_H_
