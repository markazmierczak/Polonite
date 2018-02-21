// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_MULTIPLE_H_
#define STP_BASE_MATH_MULTIPLE_H_

#include "Base/Type/Limits.h"
#include "Base/Type/Sign.h"

namespace stp {

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T roundUpToMultiple(T x, T mul) {
  static const T Max = Limits<T>::Max;

  ASSERT(mul && (x <= Max - (Max % mul)));
  return TIsPositiveValue(x)
      ? ((x + mul - 1) / mul) * mul
      : (x / mul) * mul;
}

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T roundDownToMultiple(T x, T mul) {
  static const T Min = Limits<T>::Min;

  ASSERT(mul && (x >= Min - (Min % mul)));
  return TIsPositiveValue(x)
      ? (x / mul) * mul
      : ((x - mul + 1) / mul) * mul;
}

} // namespace stp

#endif // STP_BASE_MATH_MULTIPLE_H_
