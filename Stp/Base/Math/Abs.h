// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_ABS_H_
#define STP_BASE_MATH_ABS_H_

#include "Base/Export.h"
#include "Base/Type/Sign.h"

#include <math.h>
#include <stdlib.h>

namespace stp {

template<typename T>
constexpr auto absToUnsigned(T x) {
  using UnsignedType = TMakeUnsigned<T>;
  using ResultType = TMakeUnsigned<decltype(+x)>;

  auto ux = static_cast<UnsignedType>(x);
  return isNegative(x) ? static_cast<ResultType>(0) - ux : static_cast<ResultType>(ux);
}

// Abs() is not defined for character types - they are not suited for math operations.
template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
T Abs(T x) = delete;

inline int Abs(int x) { return ::abs(x); }
inline long Abs(long x) { return ::labs(x); }
inline long long Abs(long long x) { return ::llabs(x); }

inline unsigned int Abs(unsigned int x) { return x; }
inline unsigned long Abs(unsigned long x) { return x; }
inline unsigned long long Abs(unsigned long long x) { return x; }

inline float Abs(float x) { return ::fabsf(x); }
inline double Abs(double x) { return ::fabs(x); }
inline long double Abs(long double x) { return ::fabsl(x); }

template<typename T, TEnableIf<TIsArithmetic<T>>* = nullptr>
inline bool isNear(T x, T y, T tolerance) {
  return Abs(x - y) <= tolerance;
}

BASE_EXPORT bool isNearUlp(float x, float y);
BASE_EXPORT bool isNearUlp(double x, double y);

} // namespace stp

#endif // STP_BASE_MATH_ABS_H_
