// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Codec/TextCodec.h"

#include "Base/Io/StringWriter.h"
#include "Base/Type/Hashable.h"

namespace stp {

template<typename T>
static inline List<T> BytesToStringWithCodec(BufferSpan bytes, TextEncoding codec) {
  List<T> result;
  StringTmplWriter<T> writer(&result);
  writer.Write(bytes, codec);
  return result;
}

String ToString(BufferSpan bytes, TextEncoding codec) {
  return BytesToStringWithCodec<char>(bytes, codec);
}
String16 ToString16(BufferSpan bytes, TextEncoding codec) {
  return BytesToStringWithCodec<char16_t>(bytes, codec);
}

StringSpan TextConversionFallbackException::GetName() const noexcept {
  return "TextConversionFallbackException";
}

HashCode TextEncoding::HashImpl() const {
  return Hash(this);
}

} // namespace stp
