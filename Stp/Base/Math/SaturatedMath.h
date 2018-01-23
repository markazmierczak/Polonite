// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_SATURATEDMATH_H_
#define STP_BASE_MATH_SATURATEDMATH_H_

#include "Base/Compiler/Cpu.h"
#include "Base/Math/OverflowMath.h"

namespace stp {

template<typename T, TEnableIf<TIsUnsigned<T>>* = nullptr>
inline T SaturatedNeg(T x) {
  return 0;
}

template<typename T, TEnableIf<TIsSigned<T>>* = nullptr>
inline T SaturatedNeg(T x) {
  if (UNLIKELY(x == Limits<T>::Min))
    return Limits<T>::Max;
  return -x;
}

template<typename T, TEnableIf<TIsUnsigned<T>>* = nullptr>
inline T SaturatedAbs(T x) {
  return x;
}

template<typename T, TEnableIf<TIsSigned<T>>* = nullptr>
inline T SaturatedAbs(T x) {
  return x >= 0 ? x : SaturatedNeg(x);
}

namespace detail {

template<typename T>
struct SaturatedAddHelper {
  static T Do(T x, T y) {
    using TUint = TMakeUnsigned<T>;

    T result;
    if (LIKELY(!OverflowAdd(x, y, &result)))
      return result;

    static constexpr TUint BitWidth = sizeof(T) * 8;

    TUint ux = static_cast<TUint>(x);
    TUint max = Limits<T>::Max;
    return TIsSigned<T>
        ? static_cast<T>(max + (ux >> (BitWidth-1)))
        : static_cast<T>(max);
  }
};

template<typename T, typename TEnabler = void>
struct SaturatedSubHelper {
  static T Do(T x, T y) {
    using TUint = TMakeUnsigned<T>;

    T result;
    if (LIKELY(!OverflowSub(x, y, &result)))
      return result;

    static constexpr TUint BitWidth = sizeof(T) * 8;

    TUint ux = static_cast<TUint>(x);
    TUint max = Limits<T>::Max;
    return TIsSigned<T>
        ? static_cast<T>(max + (ux >> (BitWidth-1)))
        : static_cast<T>(max);
  }
};

#if CPU(ARM32) && COMPILER(GCC)
template<>
struct SaturatedAddHelper<int32_t> {
  static int32_t Do(int32_t x, int32_t y) {
    int32_t result;
    asm("qadd %[output],%[first],%[second]"
        :   [output]  "=r"  (result)
        :   [first]   "r"   (x),
            [second]  "r"   (y));
    return result;
  }
};

template<>
struct SaturatedSubHelper<int32_t> {
  static int32_t Do(int32_t x, int32_t y) {
    int32_t result;
    asm("qsub %[output],%[first],%[second]"
        :   [output] "=r"  (result)
        :   [first]  "r"   (x),
            [second] "r"   (y));
    return result;
  }
};
#endif // CPU(ARM32) && COMPILER(GCC)

} // namespace detail

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T SaturatedAdd(T x, T y) {
  return detail::SaturatedAddHelper<T>::Do(x, y);
}

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T SaturatedSub(T x, T y) {
  return detail::SaturatedSubHelper<T>::Do(x, y);
}

} // namespace stp

#endif // STP_BASE_MATH_SATURATEDMATH_H_
