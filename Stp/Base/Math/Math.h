// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_MATH_H_
#define STP_BASE_MATH_MATH_H_

#include "Base/Type/Scalar.h"

#include <math.h>

namespace stp {

#if 1  // REGION(Basic)
inline float FusedMulAdd(float x, float y, float z) { return ::fmaf(x, y, z); }
inline double FusedMulAdd(double x, double y, double z) { return ::fma(x, y, z); }
inline long double FusedMulAdd(long double x, long double y, long double z) { return ::fmal(x, y, z); }

inline float IeeeRemainder(float x, float y) { return ::fmodf(x, y); }
inline double IeeeRemainder(double x, double y) { return ::fmod(x, y); }
inline long double IeeeRemainder(long double x, long double y) { return ::fmodl(x, y); }

namespace detail {
template<typename T>
struct DecomposeResultForFloat {
  T integral;
  T fractional;

  void Unpack(T& out_integral, T& out_fractional) const {
    out_integral = integral;
    out_fractional = fractional;
  }
};
inline float DecomposeImpl(float x, float* iptr) { return ::modff(x, iptr); }
inline double DecomposeImpl(double x, double* iptr) { return ::modf(x, iptr); }
inline long double DecomposeImpl(long double x, long double* iptr) { return ::modfl(x, iptr); }
} // namespace detail

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline detail::DecomposeResultForFloat<T> Decompose(T x) {
  detail::DecomposeResultForFloat<T> rv;
  rv.fractional = detail::DecomposeImpl(x, &rv.integral);
  return rv;
}
#endif // REGION(Basic)

#if 1  // REGION(Manipulation)
inline float CopySign(float m, float s) { return ::copysignf(m, s); }
inline double CopySign(double m, double s) { return ::copysign(m, s); }
inline long double CopySign(long double m, long double s) { return ::copysignl(m, s); }

inline float NextAfter(float from, float to) { return ::nextafterf(from, to); }
inline double NextAfter(double from, double to) { return ::nextafter(from, to); }
inline long double NextAfter(long double from, long double to) { return ::nextafterl(from, to); }
#endif // REGION(Manipulation)

#if 1  // REGION(Min/Max)
inline float IeeeMin(float x, float y) { return ::fminf(x, y); }
inline double IeeeMin(double x, double y) { return ::fmin(x, y); }
inline long double IeeeMin(long double x, long double y) { return ::fminl(x, y); }

inline float IeeeMax(float x, float y) { return ::fmaxf(x, y); }
inline double IeeeMax(double x, double y) { return ::fmax(x, y); }
inline long double IeeeMax(long double x, long double y) { return ::fmaxl(x, y); }
#endif // REGION(Min/Max)

#if 1  // REGION(Nearest Integer)
inline float Trunc(float x) { return ::truncf(x); }
inline double Trunc(double x) { return ::trunc(x); }
inline long double Trunc(long double x) { return ::truncl(x); }

inline float Floor(float x) { return ::floorf(x); }
inline double Floor(double x) { return ::floor(x); }
inline long double Floor(long double x) { return ::floorl(x); }

inline float Ceil(float x) { return ::ceilf(x); }
inline double Ceil(double x) { return ::ceil(x); }
inline long double Ceil(long double x) { return ::ceill(x); }

inline float Round(float x) { return ::roundf(x); }
inline double Round(double x) { return ::round(x); }
inline long double Round(long double x) { return ::roundl(x); }
#endif // REGION(Nearest Integer)

#if 1  // REGION(Power)
inline float Sqrt(float x) { return ::sqrtf(x); }
inline double Sqrt(double x) { return ::sqrt(x); }
inline long double Sqrt(long double x) { return ::sqrtl(x); }

inline float Cbrt(float x) { return ::cbrtf(x); }
inline double Cbrt(double x) { return ::cbrt(x); }
inline long double Cbrt(long double x) { return ::cbrtl(x); }

inline float Hypot(float x, float y) { return ::hypotf(x, y); }
inline double Hypot(double x, double y) { return ::hypot(x, y); }
inline long double Hypot(long double x, long double y) { return ::hypotl(x, y); }

inline float Pow(float base, float exp) { return ::powf(base, exp); }
inline double Pow(double base, double exp) { return ::pow(base, exp); }
inline long double Pow(long double base, long double exp) { return ::powl(base, exp); }
#endif // REGION(Power)

#if 1  // REGION(Trigonometric)
inline float Sin(float x) { return ::sinf(x); }
inline double Sin(double x) { return ::sin(x); }
inline long double Sin(long double x) { return ::sinl(x); }

inline float Cos(float x) { return ::cosf(x); }
inline double Cos(double x) { return ::cos(x); }
inline long double Cos(long double x) { return ::cosl(x); }

inline float SignBit(float x) { return signbit(x); }
inline double SignBit(double x) { return signbit(x); }
inline long double SignBit(long double x) { return signbit(x); }

namespace detail {
template<typename T>
struct SinCosResult {
  T sin;
  T cos;

  void Unpack(T& out_sin, T& out_cos) const { out_sin = sin, out_cos = cos; }
};

#if defined(__GLIBC__)
inline void SinCosImpl(float x, float* sin, float* cos) { sincosf(x, sin, cos); }
inline void SinCosImpl(double x, double* sin, double* cos) { sincos(x, sin, cos); }
inline void SinCosImpl(long double x, long double* sin, long double* cos) { sincosl(x, sin, cos); }
#endif

} // namespace detail

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline detail::SinCosResult<T> SinCos(T x) {
  #if defined(__GLIBC__)
  detail::SinCosResult<T> rv;
  detail::SinCosImpl(x, &rv.sin, &rv.cos);
  return rv;
  #else
  return detail::SinCosResult<T> { Sin(x), Cos(x) };
  #endif
}

inline float Tan(float x) { return ::tanf(x); }
inline double Tan(double x) { return ::tan(x); }
inline long double Tan(long double x) { return ::tanl(x); }

inline float Asin(float x) { return ::asinf(x); }
inline double Asin(double x) { return ::asin(x); }
inline long double Asin(long double x) { return ::asinl(x); }

inline float Acos(float x) { return ::acosf(x); }
inline double Acos(double x) { return ::acos(x); }
inline long double Acos(long double x) { return ::acosl(x); }

inline float Atan(float x) { return ::atanf(x); }
inline double Atan(double x) { return ::atan(x); }
inline long double Atan(long double x) { return ::atanl(x); }

inline float Atan2(float y, float x) { return ::atan2f(y, x); }
inline double Atan2(double y, double x) { return ::atan2(y, x); }
inline long double Atan2(long double y, long double x) { return ::atan2l(y, x); }
#endif // REGION(Trigonometric)

#if 1  // REGION(Hyperbolic)
inline float Sinh(float x) { return ::sinhf(x); }
inline double Sinh(double x) { return ::sinh(x); }
inline long double Sinh(long double x) { return ::sinhl(x); }

inline float Cosh(float x) { return ::coshf(x); }
inline double Cosh(double x) { return ::cosh(x); }
inline long double Cosh(long double x) { return ::coshl(x); }

inline float Tanh(float x) { return ::tanhf(x); }
inline double Tanh(double x) { return ::tanh(x); }
inline long double Tanh(long double x) { return ::tanhl(x); }

inline float Asinh(float x) { return ::asinhf(x); }
inline double Asinh(double x) { return ::asinh(x); }
inline long double Asinh(long double x) { return ::asinhl(x); }

inline float Acosh(float x) { return ::acoshf(x); }
inline double Acosh(double x) { return ::acosh(x); }
inline long double Acosh(long double x) { return ::acoshl(x); }

inline float Atanh(float x) { return ::atanhf(x); }
inline double Atanh(double x) { return ::atanh(x); }
inline long double Atanh(long double x) { return ::atanhl(x); }
#endif // REGION(Hyperbolic)

#if 1  // REGION(Exponential)
inline float LoadExponent(float x, int exp) { return ::ldexpf(x, exp); }
inline float LoadExponent(double x, int exp) { return ::ldexp(x, exp); }
inline float LoadExponent(long double x, int exp) { return ::ldexpl(x, exp); }

inline float Exp(float x) { return ::expf(x); }
inline double Exp(double x) { return ::exp(x); }
inline long double Exp(long double x) { return ::expl(x); }

inline float Exp2(float x) { return ::exp2f(x); }
inline double Exp2(double x) { return ::exp2(x); }
inline long double Exp2(long double x) { return ::exp2l(x); }

inline float ExpM1(float x) { return ::expm1f(x); }
inline double ExpM1(double x) { return ::expm1(x); }
inline long double ExpM1(long double x) { return ::expm1l(x); }

inline float Log(float x) { return ::logf(x); }
inline double Log(double x) { return ::log(x); }
inline long double Log(long double x) { return ::logl(x); }

inline float Log2(float x) { return ::log2f(x); }
inline double Log2(double x) { return ::log2(x); }
inline long double Log2(long double x) { return ::log2l(x); }

inline float Log10(float x) { return ::log10f(x); }
inline double Log10(double x) { return ::log10(x); }
inline long double Log10(long double x) { return ::log10l(x); }

inline float Log1P(float x) { return ::log1pf(x); }
inline double Log1P(double x) { return ::log1p(x); }
inline long double Log1P(long double x) { return ::log1pl(x); }
#endif // REGION(Exponential)

} // namespace stp

#endif // STP_BASE_MATH_MATH_H_
