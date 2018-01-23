// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_BITSSHIFT_H_
#define STP_BASE_MATH_BITSSHIFT_H_

#include "Base/Type/Sign.h"
#include "Base/Debug/Assert.h"

namespace stp {

// The behavior of left shifting signed value is undefined if last bit is toggled.
// This function casts value to unsigned before shifting.
template<typename T>
constexpr T ArithmeticShiftLeft(T x, int shift) {
  ASSERT(0 <= shift && shift < static_cast<int>(8 * sizeof(T)));
  return static_cast<T>(ToUnsigned(x) << shift);
}

template<typename T, TEnableIf<TIsInteger<T> && TIsUnsigned<T>>* = nullptr>
constexpr T RotateBitsRight(T x, int shift) {
  constexpr int MaxShift = 8 * sizeof(T);
  ASSERT(0 <= shift && shift < MaxShift);
  return shift ? ((x >> shift) | (x << (MaxShift - shift))) : x;
}

template<typename T, TEnableIf<TIsInteger<T> && TIsUnsigned<T>>* = nullptr>
constexpr T RotateBitsLeft(T x, int shift) {
  constexpr int MaxShift = 8 * sizeof(T);
  ASSERT(0 <= shift && shift < MaxShift);
  return shift ? ((x << shift) | (x >> (MaxShift - shift))) : x;
}

} // namespace stp

#endif // STP_BASE_MATH_BITSSHIFT_H_
