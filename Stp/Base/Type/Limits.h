// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_LIMITS_H_
#define STP_BASE_TYPE_LIMITS_H_

#include "Base/Type/Scalar.h"

#include <float.h>
#include <limits.h>
#include <math.h>

// NOTE: Limits<type>::Min is different from C++ Standard Library
//       and always specifies lowest() value

namespace stp {

#define COUNT_DIGITS_FOR_LIMITS sizeof(Type) * 8 - (Type(-1) < Type(0))

namespace detail {

// MSVC requires a proxy.
constexpr float getInfValueForFloat() { return __builtin_huge_valf(); }
constexpr float getNaNValueForFloat() { return __builtin_nanf("0"); }
constexpr double getInfValueForDouble() { return __builtin_huge_val(); }
constexpr double getNaNValueForDouble() { return __builtin_nan("0"); }
#if COMPILER(MSVC) && !COMPILER(CLANG)
constexpr long double getInfValueForLongDouble() { return __builtin_huge_val(); }
constexpr long double getNaNValueForLongDouble() { return __builtin_nan("0"); }
#else
constexpr long double getInfValueForLongDouble() { return __builtin_huge_vall(); }
constexpr long double getNaNValueForLongDouble() { return __builtin_nanl("0"); }
#endif

} // namespace detail

template<typename T>
struct Limits;

template<>
struct BASE_EXPORT Limits<signed char> {
  typedef signed char Type;
  static constexpr int Digits = COUNT_DIGITS_FOR_LIMITS;
  static constexpr Type Min = SCHAR_MIN;
  static constexpr Type Max = SCHAR_MAX;
};

template<>
struct BASE_EXPORT Limits<unsigned char> {
  typedef unsigned char Type;
  static constexpr int Digits = COUNT_DIGITS_FOR_LIMITS;
  static constexpr Type Min = 0;
  static constexpr Type Max = UCHAR_MAX;
};

template<>
struct BASE_EXPORT Limits<signed short> {
  typedef signed short Type;
  static constexpr int Digits = COUNT_DIGITS_FOR_LIMITS;
  static constexpr Type Min = SHRT_MIN;
  static constexpr Type Max = SHRT_MAX;
};

template<>
struct BASE_EXPORT Limits<unsigned short> {
  typedef unsigned short Type;
  static constexpr int Digits = COUNT_DIGITS_FOR_LIMITS;
  static constexpr Type Min = 0;
  static constexpr Type Max = USHRT_MAX;
};

template<>
struct BASE_EXPORT Limits<signed int> {
  typedef signed int Type;
  static constexpr int Digits = COUNT_DIGITS_FOR_LIMITS;
  static constexpr Type Min = INT_MIN;
  static constexpr Type Max = INT_MAX;
};

template<>
struct BASE_EXPORT Limits<unsigned int> {
  typedef unsigned int Type;
  static constexpr int Digits = COUNT_DIGITS_FOR_LIMITS;
  static constexpr Type Min = 0;
  static constexpr Type Max = UINT_MAX;
};

template<>
struct BASE_EXPORT Limits<signed long> {
  typedef signed long Type;
  static constexpr int Digits = COUNT_DIGITS_FOR_LIMITS;
  static constexpr Type Min = LONG_MIN;
  static constexpr Type Max = LONG_MAX;
};

template<>
struct BASE_EXPORT Limits<unsigned long> {
  typedef unsigned long Type;
  static constexpr int Digits = COUNT_DIGITS_FOR_LIMITS;
  static constexpr Type Min = 0;
  static constexpr Type Max = ULONG_MAX;
};

template<>
struct BASE_EXPORT Limits<signed long long> {
  typedef signed long long Type;
  static constexpr int Digits = COUNT_DIGITS_FOR_LIMITS;
  static constexpr Type Min = LLONG_MIN;
  static constexpr Type Max = LLONG_MAX;
};

template<>
struct BASE_EXPORT Limits<unsigned long long> {
  typedef unsigned long long Type;
  static constexpr int Digits = COUNT_DIGITS_FOR_LIMITS;
  static constexpr Type Min = 0;
  static constexpr Type Max = ULLONG_MAX;
};

#undef COUNT_DIGITS_FOR_LIMITS

template<>
struct BASE_EXPORT Limits<float> {
  typedef float Type;

  static constexpr int Digits = FLT_MANT_DIG;

  static constexpr int MinExponent = FLT_MIN_EXP;
  static constexpr int MaxExponent = FLT_MAX_EXP;

  static constexpr Type Max = FLT_MAX;
  static constexpr Type Min = -FLT_MAX;

  static constexpr Type Epsilon = FLT_EPSILON;
  static constexpr Type SmallestNormal = FLT_MIN;

  static constexpr Type Infinity = detail::getInfValueForFloat();
  static constexpr Type NaN = detail::getNaNValueForFloat();
};

template<>
struct BASE_EXPORT Limits<double> {
  typedef double Type;

  static constexpr int Digits = DBL_MANT_DIG;

  static constexpr int MinExponent = DBL_MIN_EXP;
  static constexpr int MaxExponent = DBL_MAX_EXP;

  static constexpr Type Max = DBL_MAX;
  static constexpr Type Min = -DBL_MAX;

  static constexpr Type Epsilon = DBL_EPSILON;
  static constexpr Type SmallestNormal = DBL_MIN;

  static constexpr Type Infinity = detail::getInfValueForDouble();
  static constexpr Type NaN = detail::getNaNValueForDouble();
};

template<>
struct BASE_EXPORT Limits<long double> {
  typedef long double Type;

  static constexpr int Digits = LDBL_MANT_DIG;

  static constexpr int MinExponent = LDBL_MIN_EXP;
  static constexpr int MaxExponent = LDBL_MAX_EXP;

  static constexpr Type Max = LDBL_MAX;
  static constexpr Type Min = -LDBL_MAX;

  static constexpr Type Epsilon = LDBL_EPSILON;
  static constexpr Type SmallestNormal = LDBL_MIN;

  static constexpr Type Infinity = detail::getInfValueForLongDouble();
  static constexpr Type NaN = detail::getNaNValueForLongDouble();
};

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline bool isNaN(T x) noexcept {
  return isnan(x) != 0;
}

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline bool isInfinity(T x) noexcept {
  return isinf(x) != 0;
}

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline bool isFinite(T x) noexcept {
  return isfinite(x) != 0;
}

} // namespace stp

#endif // STP_BASE_TYPE_LIMITS_H_
