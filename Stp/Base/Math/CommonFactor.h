// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_COMMONFACTOR_H_
#define STP_BASE_MATH_COMMONFACTOR_H_

#include "Base/Math/Abs.h"
#include "Base/Math/Bits.h"
#include "Base/Type/Common.h"
#include "Base/Type/Limits.h"
#include "Base/Type/Sign.h"

namespace stp {

template<typename T, typename U>
auto GreatestCommonDivisor(T ai, U bi) -> TCommon<T, U> {
  static_assert(TIsInteger<T>, "!");
  static_assert(TIsInteger<U>, "!");

  using ResultType = TCommon<T, U>;
  using UCommonType = TMakeUnsigned<ResultType>;

  UCommonType a = absToUnsigned(ai);
  UCommonType b = absToUnsigned(bi);
  if (a == 0)
    return b;
  if (b == 0)
    return a;

  int shift = FindFirstOneBit(a | b);
  a >>= FindFirstOneBit(a);
  do {
    b >>= FindFirstOneBit(b);
    if (a > b) {
      auto t = b;
      b = a;
      a = t;
    }
    b = b - a;
  } while (b != 0);
  return static_cast<ResultType>(a << shift);
}

template<typename T, typename U>
auto LeastCommonMultiple(T ai, U bi) -> TCommon<T, U> {
  if (ai == 0 || bi == 0)
    return 0;

  using ResultType = TCommon<T, U>;
  using UCommonType = TMakeUnsigned<ResultType>;

  UCommonType x = absToUnsigned(ai / GreatestCommonDivisor(ai, bi));
  UCommonType y = absToUnsigned(bi);
  ASSERT(static_cast<UCommonType>(Limits<ResultType>::Max) / x > y);
  return static_cast<ResultType>(x * y);

}

} // namespace stp

#endif // STP_BASE_MATH_COMMONFACTOR_H_
