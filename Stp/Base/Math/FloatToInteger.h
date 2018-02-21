// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_FLOATTOINTEGER_H_
#define STP_BASE_MATH_FLOATTOINTEGER_H_

#include "Base/Math/Math.h"
#include "Base/Math/SafeConversions.h"

namespace stp {

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline int mathTruncToInt(T x) {
  return assertedCast<int>(x);
}

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline int mathFloorToInt(T x) {
  return assertedCast<int>(mathFloor(x));
}

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline int mathCeilToInt(T x) {
  return assertedCast<int>(mathCeil(x));
}

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline int mathRoundToInt(T x) {
  return assertedCast<int>(mathRound(x));
}

} // namespace stp

#endif // STP_BASE_MATH_FLOATTOINTEGER_H_
