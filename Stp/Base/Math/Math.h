// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_MATH_H_
#define STP_BASE_MATH_MATH_H_

// Need to include Sign.h to properly handle mathAbs() for integer types.
#include "Base/Type/Sign.h"

#include <math.h>

namespace stp {

constexpr double MathE      = 2.718281828459045235360287471352662497e+00;
constexpr double MathPi     = 3.141592653589793238462643383279502884e+00;
constexpr double MathPhi    = 1.618033988749894848204586834365638118e+00;
constexpr double MathSqrt2  = 1.414213562373095048801688724209698078e+00;
constexpr double MathLn2    = 6.931471805599453094172321214581765680e-01;
constexpr double MathLn10   = 2.302585092994045684017991454684364207e+00;
constexpr double MathLog10E = 4.342944819032518276511289189166050822e-01;

inline float mathAbs(float x) { return ::fabsf(x); }
inline double mathAbs(double x) { return ::fabs(x); }
inline long double mathAbs(long double x) { return ::fabsl(x); }

inline float mathFusedMulAdd(float x, float y, float z) { return ::fmaf(x, y, z); }
inline double mathFusedMulAdd(double x, double y, double z) { return ::fma(x, y, z); }
inline long double mathFusedMulAdd(long double x, long double y, long double z) { return ::fmal(x, y, z); }

inline float mathRemainder(float x, float y) { return ::fmodf(x, y); }
inline double mathRemainder(double x, double y) { return ::fmod(x, y); }
inline long double mathRemainder(long double x, long double y) { return ::fmodl(x, y); }

namespace detail {
template<typename T>
struct DecomposeResultForFloat {
  T integral;
  T fractional;

  void unpack(T& out_integral, T& out_fractional) const {
    out_integral = integral;
    out_fractional = fractional;
  }
};
inline float mathDecomposeImpl(float x, float* iptr) { return ::modff(x, iptr); }
inline double mathDecomposeImpl(double x, double* iptr) { return ::modf(x, iptr); }
inline long double mathDecomposeImpl(long double x, long double* iptr) { return ::modfl(x, iptr); }
} // namespace detail

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline detail::DecomposeResultForFloat<T> mathDecompose(T x) {
  detail::DecomposeResultForFloat<T> rv;
  rv.fractional = detail::mathDecomposeImpl(x, &rv.integral);
  return rv;
}

inline float mathCopySign(float m, float s) { return ::copysignf(m, s); }
inline double mathCopySign(double m, double s) { return ::copysign(m, s); }
inline long double mathCopySign(long double m, long double s) { return ::copysignl(m, s); }

inline float mathNextAfter(float from, float to) { return ::nextafterf(from, to); }
inline double mathNextAfter(double from, double to) { return ::nextafter(from, to); }
inline long double mathNextAfter(long double from, long double to) { return ::nextafterl(from, to); }

inline float mathMin(float x, float y) { return ::fminf(x, y); }
inline double mathMin(double x, double y) { return ::fmin(x, y); }
inline long double mathMin(long double x, long double y) { return ::fminl(x, y); }

inline float mathMax(float x, float y) { return ::fmaxf(x, y); }
inline double mathMax(double x, double y) { return ::fmax(x, y); }
inline long double mathMax(long double x, long double y) { return ::fmaxl(x, y); }

inline float mathTrunc(float x) { return ::truncf(x); }
inline double mathTrunc(double x) { return ::trunc(x); }
inline long double mathTrunc(long double x) { return ::truncl(x); }

inline float mathFloor(float x) { return ::floorf(x); }
inline double mathFloor(double x) { return ::floor(x); }
inline long double mathFloor(long double x) { return ::floorl(x); }

inline float mathCeil(float x) { return ::ceilf(x); }
inline double mathCeil(double x) { return ::ceil(x); }
inline long double mathCeil(long double x) { return ::ceill(x); }

inline float mathRound(float x) { return ::roundf(x); }
inline double mathRound(double x) { return ::round(x); }
inline long double mathRound(long double x) { return ::roundl(x); }

inline float mathSqrt(float x) { return ::sqrtf(x); }
inline double mathSqrt(double x) { return ::sqrt(x); }
inline long double mathSqrt(long double x) { return ::sqrtl(x); }

inline float mathCbrt(float x) { return ::cbrtf(x); }
inline double mathCbrt(double x) { return ::cbrt(x); }
inline long double mathCbrt(long double x) { return ::cbrtl(x); }

inline float mathHypot(float x, float y) { return ::hypotf(x, y); }
inline double mathHypot(double x, double y) { return ::hypot(x, y); }
inline long double mathHypot(long double x, long double y) { return ::hypotl(x, y); }

inline float mathPow(float base, float exp) { return ::powf(base, exp); }
inline double mathPow(double base, double exp) { return ::pow(base, exp); }
inline long double mathPow(long double base, long double exp) { return ::powl(base, exp); }

inline float mathSin(float x) { return ::sinf(x); }
inline double mathSin(double x) { return ::sin(x); }
inline long double mathSin(long double x) { return ::sinl(x); }

inline float mathCos(float x) { return ::cosf(x); }
inline double mathCos(double x) { return ::cos(x); }
inline long double mathCos(long double x) { return ::cosl(x); }

inline bool mathHasSignBit(float x) { return signbit(x) != 0; }
inline bool mathHasSignBit(double x) { return signbit(x) != 0; }
inline bool mathHasSignBit(long double x) { return signbit(x) != 0; }

namespace detail {
template<typename T>
struct SinCosResult {
  T sin;
  T cos;

  void unpack(T& out_sin, T& out_cos) const { out_sin = sin, out_cos = cos; }
};

#if defined(__GLIBC__)
inline void mathSinCosImpl(float x, float* sin, float* cos) { sincosf(x, sin, cos); }
inline void mathSinCosImpl(double x, double* sin, double* cos) { sincos(x, sin, cos); }
inline void mathSinCosImpl(long double x, long double* sin, long double* cos) { sincosl(x, sin, cos); }
#endif

} // namespace detail

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
inline detail::SinCosResult<T> mathSinCos(T x) {
  #if defined(__GLIBC__)
  detail::SinCosResult<T> rv;
  detail::mathSinCosImpl(x, &rv.sin, &rv.cos);
  return rv;
  #else
  return detail::SinCosResult<T> { mathSin(x), mathCos(x) };
  #endif
}

inline float mathTan(float x) { return ::tanf(x); }
inline double mathTan(double x) { return ::tan(x); }
inline long double mathTan(long double x) { return ::tanl(x); }

inline float mathAsin(float x) { return ::asinf(x); }
inline double mathAsin(double x) { return ::asin(x); }
inline long double mathAsin(long double x) { return ::asinl(x); }

inline float mathAcos(float x) { return ::acosf(x); }
inline double mathAcos(double x) { return ::acos(x); }
inline long double mathAcos(long double x) { return ::acosl(x); }

inline float mathAtan(float x) { return ::atanf(x); }
inline double mathAtan(double x) { return ::atan(x); }
inline long double mathAtan(long double x) { return ::atanl(x); }

inline float mathAtan2(float y, float x) { return ::atan2f(y, x); }
inline double mathAtan2(double y, double x) { return ::atan2(y, x); }
inline long double mathAtan2(long double y, long double x) { return ::atan2l(y, x); }

inline float mathSinh(float x) { return ::sinhf(x); }
inline double mathSinh(double x) { return ::sinh(x); }
inline long double mathSinh(long double x) { return ::sinhl(x); }

inline float mathCosh(float x) { return ::coshf(x); }
inline double mathCosh(double x) { return ::cosh(x); }
inline long double mathCosh(long double x) { return ::coshl(x); }

inline float mathTanh(float x) { return ::tanhf(x); }
inline double mathTanh(double x) { return ::tanh(x); }
inline long double mathTanh(long double x) { return ::tanhl(x); }

inline float mathAsinh(float x) { return ::asinhf(x); }
inline double mathAsinh(double x) { return ::asinh(x); }
inline long double mathAsinh(long double x) { return ::asinhl(x); }

inline float mathAcosh(float x) { return ::acoshf(x); }
inline double mathAcosh(double x) { return ::acosh(x); }
inline long double mathAcosh(long double x) { return ::acoshl(x); }

inline float mathAtanh(float x) { return ::atanhf(x); }
inline double mathAtanh(double x) { return ::atanh(x); }
inline long double mathAtanh(long double x) { return ::atanhl(x); }

inline float mathLoadExponent(float x, int exp) { return ::ldexpf(x, exp); }
inline float mathLoadExponent(double x, int exp) { return ::ldexp(x, exp); }
inline float mathLoadExponent(long double x, int exp) { return ::ldexpl(x, exp); }

inline float mathExp(float x) { return ::expf(x); }
inline double mathExp(double x) { return ::exp(x); }
inline long double mathExp(long double x) { return ::expl(x); }

inline float mathExp2(float x) { return ::exp2f(x); }
inline double mathExp2(double x) { return ::exp2(x); }
inline long double mathExp2(long double x) { return ::exp2l(x); }

inline float mathExpm1(float x) { return ::expm1f(x); }
inline double mathExpm1(double x) { return ::expm1(x); }
inline long double mathExpm1(long double x) { return ::expm1l(x); }

inline float mathLog(float x) { return ::logf(x); }
inline double mathLog(double x) { return ::log(x); }
inline long double mathLog(long double x) { return ::logl(x); }

inline float mathLog2(float x) { return ::log2f(x); }
inline double mathLog2(double x) { return ::log2(x); }
inline long double mathLog2(long double x) { return ::log2l(x); }

inline float mathLog10(float x) { return ::log10f(x); }
inline double mathLog10(double x) { return ::log10(x); }
inline long double mathLog10(long double x) { return ::log10l(x); }

inline float mathLog1p(float x) { return ::log1pf(x); }
inline double mathLog1p(double x) { return ::log1p(x); }
inline long double mathLog1p(long double x) { return ::log1pl(x); }

} // namespace stp

#endif // STP_BASE_MATH_MATH_H_
