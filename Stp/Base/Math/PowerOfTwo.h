// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

// Documented in Bits.rst

#ifndef STP_BASE_MATH_POWEROFTWO_H_
#define STP_BASE_MATH_POWEROFTWO_H_

#include "Base/Math/Bits.h"
#include "Base/Type/Limits.h"

namespace stp {

// Returns the integer i such as 2^i <= n < 2^(i+1).
// Assumes |x| is not zero.
template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline int Log2Floor(T x) {
  ASSERT(x >= 0);
  return findLastOneBit(x);
}

// Returns the integer i such as 2^(i-1) < n <= 2^i.
// Assumes |x| is not zero.
template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline int Log2Ceil(T x) {
  ASSERT(x >= 0);
  return (x != 1) ? (1 + Log2Floor(x - 1)) : 0;
}

// Returns true iff x is a power of 2.
template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr bool IsPowerOfTwo(T x) {
  return x > 0 && !(x & (x-1));
}

// Returns which power of two is |x|.
// |x| must be a power of 2.
template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline int WhichPowerOfTwo(T x) {
  ASSUME(x > 0);
  ASSERT(IsPowerOfTwo(x));
  return findFirstOneBit(x);
}

// Returns the smallest power of two which is >= |x|.
// If you pass in a number that is already a power of two, it is returned as is.
// If |x| is lower or equal to zero, one is returned.
// |x| must be lower than maximum representable power of two.
template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T RoundUpToPowerOfTwo(T x) {
  if (x > 1) {
    int p = findLastOneBit(x - 1) + 1;
    ASSERT(p <= Limits<T>::Digits);
    return T(1) << p;
  }
  return 1;
}

// Returns the greatest power of two which is <= |x|.
// If you pass in a number that is already a power of two, it is returned as is.
template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T RoundDownToPowerOfTwo(T x) {
  ASSUME(x > 0);
  return T(1) << findLastOneBit(x);
}

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline int TryLog2(T x) {
  return IsPowerOfTwo(x) ? WhichPowerOfTwo(x) : -1;
}

} // namespace stp

#endif // STP_BASE_MATH_POWEROFTWO_H_
