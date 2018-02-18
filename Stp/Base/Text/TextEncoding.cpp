// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncoding.h"

#include "Base/Io/StringWriter.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Type/Hashable.h"

namespace stp {

// FIXME
//String ToString(BufferSpan bytes, TextEncoding encoding) {
//  String result;
//  StringWriter writer(&result);
//  writer.Write(bytes, encoding);
//  return result;
//}

HashCode TextEncoding::HashImpl() const noexcept {
  return partialHash(this);
}

namespace detail {

constexpr TextEncodingData BuildNull() {
  TextEncodingData codec;
  codec.name = "undefined";
  return codec;
}

constexpr const TextEncodingData UndefinedTextEncodingData = BuildNull();

} // namespace detail

static inline StringSpan RemoveNonAlphaNumericPrefix(StringSpan s) {
  while (!s.IsEmpty() && !isAlphaNumericAscii(s.getFirst()))
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

    if (toUpperAscii(lhs.getFirst()) != toUpperAscii(rhs.getFirst()))
      return false;

    lhs.RemovePrefix(1);
    rhs.RemovePrefix(1);
  }
  return lhs.IsEmpty() && rhs.IsEmpty();
}

} // namespace stp
