// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_ASCIICHAR_H_
#define STP_BASE_TEXT_ASCIICHAR_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Scalar.h"

namespace stp {

namespace detail {

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool isInRangeAscii(T c, char lo, char hi) {
  return (charCast<char32_t>(c) - lo) <= static_cast<char32_t>(hi - lo);
}

} // namespace detail

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool isAscii(T c) {
  return charCast<char32_t>(c) <= 0x7F;
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool isUpperAscii(T c) {
  return detail::isInRangeAscii(c, 'A', 'Z');
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool isLowerAscii(T c) {
  return detail::isInRangeAscii(c, 'a', 'z');
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool isSpaceAscii(T c) {
  return charCast<char32_t>(c) <= ' ' && (c == ' ' || c == '\r' || c == '\n' || c == '\t');
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool isAlphaAscii(T c) {
  return isLowerAscii(c) || isUpperAscii(c);
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool isDigitAscii(T c) {
  return detail::isInRangeAscii(c, '0', '9');
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool isAlphaNumericAscii(T c) {
  return isAlphaAscii(c) || isDigitAscii(c);
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool isPrintAscii(T c) {
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

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool isHexDigit(T c) {
  return isDigitAscii(c) ||
      detail::isInRangeAscii(c, 'A', 'F') ||
      detail::isInRangeAscii(c, 'a', 'f');
}

constexpr char32_t nibbleToHexDigitUpper(int n) {
  ASSUME(n < 16);
  return n < 10 ? (n + '0') : ((n - 10) + 'A');
}

constexpr char32_t nibbleToHexDigitLower(int n) {
  ASSUME(n < 16);
  return n < 10 ? (n + '0') : ((n - 10) + 'a');
}

constexpr char32_t nibbleToHexDigit(int n, bool uppercase) {
  return uppercase ? nibbleToHexDigitUpper(n) : nibbleToHexDigitLower(n);
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
[[nodiscard]] constexpr int tryParseHexDigit(T c) {
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
