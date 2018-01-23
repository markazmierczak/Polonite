// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_ASCIICHAR_H_
#define STP_BASE_TEXT_ASCIICHAR_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Scalar.h"

namespace stp {

namespace detail {

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool IsInRangeAscii(T c, char lo, char hi) {
  return (char_cast<char32_t>(c) - lo) <= static_cast<char32_t>(hi - lo);
}

} // namespace detail

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool IsAscii(T c) {
  return char_cast<char32_t>(c) <= 0x7F;
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool IsUpperAscii(T c) {
  return detail::IsInRangeAscii(c, 'A', 'Z');
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool IsLowerAscii(T c) {
  return detail::IsInRangeAscii(c, 'a', 'z');
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool IsSpaceAscii(T c) {
  return char_cast<char32_t>(c) <= ' ' && (c == ' ' || c == '\r' || c == '\n' || c == '\t');
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool IsAlphaAscii(T c) {
  return IsLowerAscii(c) || IsUpperAscii(c);
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool IsDigitAscii(T c) {
  return detail::IsInRangeAscii(c, '0', '9');
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool IsAlphaNumericAscii(T c) {
  return IsAlphaAscii(c) || IsDigitAscii(c);
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool IsPrintAscii(T c) {
  return '\x20' <= c && c <= '\x7E';
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr T ToLowerAscii(T c) {
  return IsUpperAscii(c) ? char_cast<T>(c + ('a' - 'A')) : c;
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr T ToUpperAscii(T c) {
  return IsLowerAscii(c) ? char_cast<T>(c + ('A' - 'a')) : c;
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
constexpr bool IsHexDigit(T c) {
  return IsDigitAscii(c) ||
      detail::IsInRangeAscii(c, 'A', 'F') ||
      detail::IsInRangeAscii(c, 'a', 'f');
}

constexpr char32_t NibbleToHexDigitUpper(int n) {
  ASSUME(n < 16);
  return n < 10 ? (n + '0') : ((n - 10) + 'A');
}

constexpr char32_t NibbleToHexDigitLower(int n) {
  ASSUME(n < 16);
  return n < 10 ? (n + '0') : ((n - 10) + 'a');
}

constexpr char32_t NibbleToHexDigit(int n, bool uppercase) {
  return uppercase ? NibbleToHexDigitUpper(n) : NibbleToHexDigitLower(n);
}

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
[[nodiscard]] constexpr int TryParseHexDigit(T c) {
  if (IsDigitAscii(c))
    return c - '0';
  if (detail::IsInRangeAscii(c, 'A', 'F'))
    return c - 'A' + 10;
  if (detail::IsInRangeAscii(c, 'a', 'f'))
    return c - 'a' + 10;
  return -1;
}

} // namespace stp

#endif // STP_BASE_TEXT_ASCIICHAR_H_
