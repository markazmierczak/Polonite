// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncoding.h"

#include "Base/Io/StringWriter.h"
#include "Base/Type/Hashable.h"

namespace stp {

// FIXME
//template<typename T>
//static inline List<T> BytesToStringWithCodec(BufferSpan bytes, TextEncoding codec) {
//  List<T> result;
//  StringTmplWriter<T> writer(&result);
//  writer.Write(bytes, codec);
//  return result;
//}
//
//String ToString(BufferSpan bytes, TextEncoding codec) {
//  return BytesToStringWithCodec<char>(bytes, codec);
//}
//String16 ToString16(BufferSpan bytes, TextEncoding codec) {
//  return BytesToStringWithCodec<char16_t>(bytes, codec);
//}

StringSpan TextConversionFallbackException::GetName() const noexcept {
  return "TextConversionFallbackException";
}

HashCode TextEncoding::HashImpl() const noexcept {
  return Hash(this);
}

namespace detail {

static constexpr TextCodecVtable UndefinedVtable = {
  nullptr, nullptr,
  nullptr, nullptr,
};

constexpr const TextCodec UndefinedTextCodec = BuildTextCodec("undefined", UndefinedVtable);

} // namespace detail

static inline StringSpan RemoveNonAlphaNumericPrefix(StringSpan s) {
  while (!s.IsEmpty() && !IsAlphaNumericAscii(s.GetFirst()))
    s.RemovePrefix(1);
  return s;
}

bool TextEncoding::AreNamesMatching(StringSpan lhs, StringSpan rhs) noexcept {
  if (lhs == rhs)
    return true;

  while (!lhs.IsEmpty() && !rhs.IsEmpty()) {
    lhs = RemoveNonAlphaNumericPrefix(lhs);
    rhs = RemoveNonAlphaNumericPrefix(rhs);
    if (lhs.IsEmpty() || rhs.IsEmpty())
      break;

    if (ToUpperAscii(lhs.GetFirst()) != ToUpperAscii(rhs.GetFirst()))
      return false;

    lhs.RemovePrefix(1);
    rhs.RemovePrefix(1);
  }
  return lhs.IsEmpty() && rhs.IsEmpty();
}

} // namespace stp
