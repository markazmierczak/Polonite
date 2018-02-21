// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_SATURATEDMATH_H_
#define STP_BASE_MATH_SATURATEDMATH_H_

#include "Base/Compiler/Cpu.h"
#include "Base/Math/OverflowMath.h"

namespace stp {

template<typename T, TEnableIf<TIsUnsigned<T>>* = nullptr>
inline T saturatedNeg(T x) {
  return 0;
}

template<typename T, TEnableIf<TIsSigned<T>>* = nullptr>
inline T saturatedNeg(T x) {
  if (UNLIKELY(x == Limits<T>::Min))
    return Limits<T>::Max;
  return -x;
}

template<typename T, TEnableIf<TIsUnsigned<T>>* = nullptr>
inline T saturatedAbs(T x) {
  return x;
}

template<typename T, TEnableIf<TIsSigned<T>>* = nullptr>
inline T saturatedAbs(T x) {
  return x >= 0 ? x : saturatedNeg(x);
}

namespace detail {

template<typename T>
struct SaturatedAddHelper {
  static T add(T x, T y) {
    using TUint = TMakeUnsigned<T>;

    T result;
    if (LIKELY(!overflowAdd(x, y, &result)))
      return result;

    constexpr TUint BitWidth = sizeof(T) * 8;

    TUint ux = static_cast<TUint>(x);
    TUint max = Limits<T>::Max;
    return TIsSigned<T>
        ? static_cast<T>(max + (ux >> (BitWidth-1)))
        : static_cast<T>(max);
  }
};

template<typename T, typename TEnabler = void>
struct SaturatedSubHelper {
  static T sub(T x, T y) {
    using TUint = TMakeUnsigned<T>;

    T result;
    if (LIKELY(!overflowSub(x, y, &result)))
      return result;

    constexpr TUint BitWidth = sizeof(T) * 8;

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
  static int32_t add(int32_t x, int32_t y) {
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
  static int32_t sub(int32_t x, int32_t y) {
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
inline T saturatedAdd(T x, T y) {
  return detail::SaturatedAddHelper<T>::add(x, y);
}

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T saturatedSub(T x, T y) {
  return detail::SaturatedSubHelper<T>::sub(x, y);
}

} // namespace stp

#endif // STP_BASE_MATH_SATURATEDMATH_H_
