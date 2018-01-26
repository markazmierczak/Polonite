// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_FLOATTOINTEGER_H_
#define STP_BASE_MATH_FLOATTOINTEGER_H_

#include "Base/Math/Math.h"
#include "Base/Math/SafeConversions.h"

namespace stp {

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline int TruncToInt(T x) {
  return AssertedCast<int>(x);
}

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline int FloorToInt(T x) {
  return AssertedCast<int>(Floor(x));
}

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline int CeilToInt(T x) {
  return AssertedCast<int>(Ceil(x));
}

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline int RoundToInt(T x) {
  return AssertedCast<int>(Round(x));
}

} // namespace stp

#endif // STP_BASE_MATH_FLOATTOINTEGER_H_
