// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_FORMATINTEGER_H_
#define STP_BASE_TEXT_FORMATINTEGER_H_

#include "Base/Containers/Array.h"
#include "Base/Type/Sign.h"
#include "Base/Text/AsciiChar.h"

namespace stp {

// Do not use these functions in any UI unless it's NOT localized on purpose.
//
// Converters create the string in a temporary buffer, write it back to front, and
// then return the substr of what we ended up using.

// log10(2) ~= 0.3 bytes needed per bit or per byte log10(2**8) ~= 2.4.
// So round up to allocate 3 output characters per byte, plus 1 for '-'.
template<typename T>
using FormatIntegerBuffer = Array<char, 3 * sizeof(T) + TIsSigned<T>>;

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr StringSpan FormatInteger(T input, MutableStringSpan buffer) {
  using UnsignedType = TMakeUnsigned<T>;

  char* end = buffer.data() + buffer.size();
  char* begin = end;

  // We need to switch to the unsigned type when negating the value since
  // abs(INT_MIN) == INT_MAX + 1.
  bool negative = IsNegative(input);
  auto value = negative ? static_cast<UnsignedType>(0) - input : static_cast<UnsignedType>(input);
  do {
    --begin;
    *begin = static_cast<char>((value % 10) + '0');
    value /= 10;
  } while (value);
  if (negative) {
    --begin;
    *begin = '-';
  }
  ASSERT(begin >= buffer.data());
  return StringSpan(begin, end - begin);
}

template<typename T>
using FormatHexIntegerBuffer = char[2 * sizeof(T) + TIsSigned<T>];

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr StringSpan FormatHexInteger(
    T input, MutableStringSpan buffer, bool uppercase = true) {
  using UnsignedType = TMakeUnsigned<T>;

  char* end = buffer.data() + buffer.size();
  char* begin = end;

  bool negative = IsNegative(input);
  auto value = negative ? static_cast<UnsignedType>(0) - input : static_cast<UnsignedType>(input);
  do {
    --begin;
    if (uppercase)
      *begin = nibbleToHexDigit(value & 0xF, uppercase);
    value >>= 4;
  } while (value);
  if (negative) {
    --begin;
    *begin = '-';
  }
  ASSERT(begin >= buffer.data());
  return StringSpan(begin, end - begin);
}

template<typename T>
using FormatOctalIntegerBuffer = char[(8 * sizeof(T) + 2) / 3 + TIsSigned<T>];

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr StringSpan FormatOctalInteger(T input, MutableStringSpan buffer) {
  using UnsignedType = TMakeUnsigned<T>;

  char* end = buffer.data() + buffer.size();
  char* begin = end;

  bool negative = IsNegative(input);
  auto value = negative ? static_cast<UnsignedType>(0) - input : static_cast<UnsignedType>(input);
  do {
    --begin;
    *begin = static_cast<char>((value & 0x7) + '0');
    value >>= 3;
  } while (value);
  if (negative) {
    --begin;
    *begin = '-';
  }
  ASSERT(begin >= buffer.data());
  return StringSpan(begin, end - begin);
}

} // namespace stp

#endif // STP_BASE_TEXT_FORMATINTEGER_H_
