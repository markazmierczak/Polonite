// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_OVERFLOWMATH_H_
#define STP_BASE_MATH_OVERFLOWMATH_H_

#include "Base/Debug/Assert.h"
#include "Base/Math/Abs.h"
#include "Base/Type/Limits.h"
#include "Base/Type/Sign.h"
#include "Base/Type/IntegerSelection.h"

// Probe for builtin math overflow support on Clang and version check on GCC.
#define HAVE_OVERFLOW_BUILTINS (COMPILER(GCC) || COMPILER(CLANG))

namespace stp {

template<typename T, TEnableIf<TIsUnsigned<T>>* = nullptr>
inline bool OverflowNeg(T x, T* presult) {
  *presult = static_cast<T>(0) - x;
  return x != 0;
}

template<typename T, TEnableIf<TIsSigned<T>>* = nullptr>
inline bool OverflowNeg(T x, T* presult) {
  using TUint = TMakeUnsigned<T>;

  TUint ux = static_cast<TUint>(x);
  TUint uresult = static_cast<TUint>(0) - ux;
  *presult = static_cast<T>(uresult);
  return x == Limits<T>::Min;
}

template<typename T, TEnableIf<TIsUnsigned<T>>* = nullptr>
inline bool OverflowAbs(T x, T* presult) {
  *presult = x;
  return false;
}

template<typename T, TEnableIf<TIsSigned<T>>* = nullptr>
inline bool OverflowAbs(T x, T* presult) {
  if (x >= 0) {
    *presult = x;
    return false;
  }
  return OverflowNeg(x, presult);
}

#if HAVE_OVERFLOW_BUILTINS

template<typename T>
static bool OverflowAdd(T x, T y, T* result) {
  static_assert(TIsInteger<T>, "!");
  return __builtin_add_overflow(x, y, result);
}

template<typename T>
static bool OverflowSub(T x, T y, T* result) {
  return __builtin_sub_overflow(x, y, result);
}

template<typename T>
static bool OverflowMul(T x, T y, T* result) {
  return __builtin_mul_overflow(x, y, result);
}

#else
template<typename T, TEnableIf<sizeof(T) < sizeof(uintmax_t)>* = nullptr>
inline bool OverflowAdd(T x, T y, T* presult) {
  using TWiden = TMakeInteger<TIsSigned<T>, sizeof(T) * 2>;
  TWiden wresult = static_cast<TWiden>(x) + y;
  T result = static_cast<T>(wresult);
  *presult = result;
  return wresult != result;
}

template<typename T, TEnableIf<sizeof(T) >= sizeof(uintmax_t)>* = nullptr>
inline bool OverflowAdd(T x, T y, T* presult) {
  static_assert(TIsInteger<T>, "!");
  using TUint = TMakeUnsigned<T>;

  TUint ux = static_cast<TUint>(x);
  TUint uy = static_cast<TUint>(y);
  TUint uresult = ux + uy;
  *presult = static_cast<T>(uresult);
  return TIsSigned<T>
      ? static_cast<T>((uresult ^ ux) & (uresult ^ uy)) < 0
      : uresult < uy;
}

template<typename T, TEnableIf<sizeof(T) < sizeof(uintmax_t)>* = nullptr>
inline bool OverflowSub(T x, T y, T* presult) {
  using TWiden = TMakeInteger<TIsSigned<T>, sizeof(T) * 2>;
  TWiden wresult = static_cast<TWiden>(x) - y;
  T result = static_cast<T>(wresult);
  *presult = result;
  return wresult != result;
}

template<typename T, TEnableIf<sizeof(T) >= sizeof(uintmax_t)>* = nullptr>
inline bool OverflowSub(T x, T y, T* presult) {
  static_assert(TIsInteger<T>, "!");
  using TUint = TMakeUnsigned<T>;

  TUint ux = static_cast<TUint>(x);
  TUint uy = static_cast<TUint>(y);
  TUint uresult = ux - uy;
  *presult = static_cast<T>(uresult);
  return TIsSigned<T> ? static_cast<T>((uresult ^ ux) & (ux ^ uy)) < 0
                        : x < y;
}

template<typename T, TEnableIf<sizeof(T) < sizeof(uintmax_t)>* = nullptr>
inline bool OverflowMul(T x, T y, T* presult) {
  static_assert(TIsInteger<T>, "type must be integral");
  using TWiden = TMakeInteger<TIsSigned<T>, sizeof(T) * 2>;
  TWiden wresult = static_cast<TWiden>(x) * y;
  T result = static_cast<T>(wresult);
  *presult = result;
  return wresult != result;
}

template<typename T, TEnableIf<sizeof(T) >= sizeof(uintmax_t)>* = nullptr>
inline bool OverflowMul(T x, T y, T* presult) {
  static_assert(TIsInteger<T>, "!");
  using TUint = TMakeUnsigned<T>;

  // Since the value of x*y is potentially undefined if we have a signed type,
  // we compute it using the unsigned type of the same size.
  TUint ux = AbsToUnsigned(x);
  TUint uy = AbsToUnsigned(y);
  TUint uresult = static_cast<TUint>(ux * uy);
  bool is_negative = IsNegative(x ^ y);
  *presult = is_negative ? static_cast<TUint>(0) - uresult : uresult;
  // We have a fast out for unsigned identity or zero on the second operand.
  // After that it's an unsigned overflow check on the absolute value, with
  // a +1 bound for a negative result.
  return uy > TUint(TIsUnsigned<T> || is_negative) &&
         ux > (Limits<T>::Max + TUint(is_negative)) / uy;
}

#endif // !HAVE_OVERFLOW_BUILTINS

template<typename T>
inline bool OverflowDiv(T x, T y, T* presult) {
  ASSERT(y != 0);
  if ((TIsUnsigned<T> || x != Limits<T>::Min || y != static_cast<T>(-1))) {
    *presult = x / y;
    return false;
  }
  return true;
}

namespace detail {

template<typename T, typename U, bool TIsSigned = TIsSigned<T>>
struct OverflowLShiftOp {
  static bool Do(T x, U shift, T* presult) {
    *presult = x << shift;
    return false;
  }
};

template<typename T, typename U>
struct OverflowLShiftOp<T, U, true> {
  static bool Do(T x, U shift, T* presult) {
    using UnsignedType = TMakeUnsigned<T>;
    T rv = static_cast<T>(static_cast<UnsignedType>(x) << shift);
    bool overflow = (rv >> shift) != x;
    *presult = rv;
    return overflow;
  }
};

} // namespace detail

template<typename T, typename U>
inline bool OverflowLShift(T x, U shift, T* presult) {
  ASSERT(!IsNegative(shift));
  ASSERT(shift < static_cast<U>(8 * sizeof(T)));
  return detail::OverflowLShiftOp<T, U>::Do(x, shift, presult);
}

#undef HAVE_OVERFLOW_BUILTINS

} // namespace stp

#endif // STP_BASE_MATH_OVERFLOWMATH_H_
