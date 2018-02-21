// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_BITS_H_
#define STP_BASE_MATH_BITS_H_

#include "Base/Debug/Assert.h"
#include "Base/Math/BitsImpl.h"

namespace stp {

template<typename T>
inline int findFirstOneBit(T x) {
  return detail::findFirstOneBitImpl(toUnsigned(x));
}

template<typename T>
inline int findLastOneBit(T x) {
  return detail::findLastOneBitImpl(toUnsigned(x));
}

template<typename T>
inline T extractFirstOneBit(T x) {
  return static_cast<T>(detail::extractFirstOneBitImpl(toUnsigned(x)));
}

template<typename T>
inline T extractLastOneBit(T x) {
  using UnsignedType = TMakeUnsigned<T>;
  return x ? static_cast<T>(UnsignedType(1) << findLastOneBit(x)) : 0;
}

template<typename T>
inline int countTrailingZeroBits(T x) {
  return detail::countTrailingZeroBitsImpl(toUnsigned(x));
}

template<typename T>
inline int countLeadingZeroBits(T x) {
  return detail::countLeadingZeroBitsImpl(toUnsigned(x));
}

template<typename T>
inline bool getBitsParity(T x) {
  return detail::getBitsParityImpl(toUnsigned(x));
}

template<typename T>
inline int countBitsPopulation(T x) {
  return detail::countBitsPopulationImpl(toUnsigned(x));
}

template<typename T>
inline T reverseBits(T x) {
  return detail::reverseBitsImpl(toUnsigned(x));
}

// The behavior of left shifting signed value is undefined if last bit is toggled.
// This function casts value to unsigned before shifting.
template<typename T>
constexpr T arithmeticShiftLeft(T x, int shift) {
  ASSERT(0 <= shift && shift < static_cast<int>(8 * sizeof(T)));
  return static_cast<T>(toUnsigned(x) << shift);
}

template<typename T, TEnableIf<TIsInteger<T> && TIsUnsigned<T>>* = nullptr>
constexpr T rotateBitsRight(T x, int shift) {
  constexpr int MaxShift = 8 * sizeof(T);
  ASSERT(0 <= shift && shift < MaxShift);
  return shift ? ((x >> shift) | (x << (MaxShift - shift))) : x;
}

template<typename T, TEnableIf<TIsInteger<T> && TIsUnsigned<T>>* = nullptr>
constexpr T rotateBitsLeft(T x, int shift) {
  constexpr int MaxShift = 8 * sizeof(T);
  ASSERT(0 <= shift && shift < MaxShift);
  return shift ? ((x << shift) | (x >> (MaxShift - shift))) : x;
}

} // namespace stp

#endif // STP_BASE_MATH_BITS_H_
