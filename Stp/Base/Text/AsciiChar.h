// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_ASCIICHAR_H_
#define STP_BASE_TEXT_ASCIICHAR_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Scalar.h"

namespace stp {

namespace detail {

constexpr bool isInRangeAscii(char32_t c, char lo, char hi) {
  ASSERT(lo <= hi);
  return (c - lo) <= static_cast<char32_t>(hi - lo);
}

} // namespace detail

constexpr bool isAscii(char32_t c) {
  return c <= 0x7F;
}

constexpr bool isUpperAscii(char32_t c) {
  return detail::isInRangeAscii(c, 'A', 'Z');
}

constexpr bool isLowerAscii(char32_t c) {
  return detail::isInRangeAscii(c, 'a', 'z');
}

constexpr bool isSpaceAscii(char32_t c) {
  return c <= ' ' && (c == ' ' || c == '\r' || c == '\n' || c == '\t');
}

constexpr bool isAlphaAscii(char32_t c) {
  return isLowerAscii(c) || isUpperAscii(c);
}

constexpr bool isDigitAscii(char32_t c) {
  return detail::isInRangeAscii(c, '0', '9');
}

constexpr bool isAlphaNumericAscii(char32_t c) {
  return isAlphaAscii(c) || isDigitAscii(c);
}

constexpr bool isPrintAscii(char32_t c) {
  return '\x20' <= c && c <= '\x7E';
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr T toLowerAscii(T c) {
  return isUpperAscii(c) ? charCast<T>(c + ('a' - 'A')) : c;
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr T toUpperAscii(T c) {
  return isLowerAscii(c) ? charCast<T>(c + ('A' - 'a')) : c;
}

constexpr bool isHexDigit(char32_t c) {
  return isDigitAscii(c) ||
      detail::isInRangeAscii(c, 'A', 'F') ||
      detail::isInRangeAscii(c, 'a', 'f');
}

constexpr char nibbleToHexDigitUpper(int n) {
  ASSERT(0 <= n && n < 16);
  return n < 10 ? (n + '0') : ((n - 10) + 'A');
}

constexpr char nibbleToHexDigitLower(int n) {
  ASSERT(0 <= n && n < 16);
  return n < 10 ? (n + '0') : ((n - 10) + 'a');
}

constexpr char nibbleToHexDigit(int n, bool uppercase) {
  return uppercase ? nibbleToHexDigitUpper(n) : nibbleToHexDigitLower(n);
}

[[nodiscard]] constexpr int tryParseHexDigit(char32_t c) {
  if (isDigitAscii(c))
    return c - '0';
  if (detail::isInRangeAscii(c, 'A', 'F'))
    return c - 'A' + 10;
  if (detail::isInRangeAscii(c, 'a', 'f'))
    return c - 'a' + 10;
  return -1;
}

} // namespace stp

#endif // STP_BASE_TEXT_ASCIICHAR_H_
