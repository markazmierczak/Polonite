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

HashCode TextEncoding::HashImpl() const {
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
  while (!s.isEmpty() && !isAlphaNumericAscii(s[0]))
    s.removePrefix(1);
  return s;
}

bool TextEncoding::AreNamesMatching(StringSpan lhs, StringSpan rhs) {
  if (lhs == rhs)
    return true;

  while (!lhs.isEmpty() && !rhs.isEmpty()) {
    lhs = RemoveNonAlphaNumericPrefix(lhs);
    rhs = RemoveNonAlphaNumericPrefix(rhs);
    if (lhs.isEmpty() || rhs.isEmpty())
      break;

    if (toUpperAscii(lhs[0]) != toUpperAscii(rhs[0]))
      return false;

    lhs.removePrefix(1);
    rhs.removePrefix(1);
  }
  return lhs.isEmpty() && rhs.isEmpty();
}

} // namespace stp
