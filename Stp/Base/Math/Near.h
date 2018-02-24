// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_NEAR_H_
#define STP_BASE_MATH_NEAR_H_

#include "Base/Math/Math.h"

namespace stp {

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr bool isNear(T x, T y, T tolerance) {
  return mathAbs(x - y) <= tolerance;
}

BASE_EXPORT bool isNearUlp(float x, float y);
BASE_EXPORT bool isNearUlp(double x, double y);

} // namespace stp

#endif // STP_BASE_MATH_NEAR_H_
