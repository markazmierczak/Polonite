// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_FIXED_H_
#define STP_BASE_MATH_FIXED_H_

#include "Base/Containers/SpanFwd.h"
#include "Base/Math/Safe.h"

namespace stp {

namespace detail {

BASE_EXPORT int32_t fixedSqrt(int32_t value, int count);
BASE_EXPORT int64_t longFixedSqrt(int64_t value, int count);

BASE_EXPORT int32_t fixedRsqrt16(int32_t x);

BASE_EXPORT void fixedFormat(TextWriter& writer, int32_t value, int point);
BASE_EXPORT void fixedFormat(TextWriter& writer, const StringSpan& opts, int32_t value, int point);

} // namespace detail

template<int P>
class Fixed;

// Fixed16 is an alias for Fixed16_16 as it is the mostly used case.
typedef Fixed<16> Fixed16;
typedef Fixed<16> Fixed16_16;
typedef Fixed<8> Fixed24_8;
typedef Fixed<6> Fixed26_6;

template<int P>
class Fixed {
 public:
  static_assert(0 < P && P < 31, "invalid point");

  typedef int32_t BitsType;
  typedef int64_t WideBitsType;

  static constexpr BitsType OneBitValue = 1 << P;
  static constexpr BitsType HalfBitValue = 1 << (P - 1);

  static constexpr BitsType FractionBitMask = OneBitValue - 1;

 private:
  struct FromBitsTag {};
  constexpr Fixed(BitsType bits, FromBitsTag) : bits_(bits) {}
 public:

  static constexpr Fixed fromBits(BitsType bits) { return Fixed(bits, FromBitsTag()); }
  ALWAYS_INLINE constexpr BitsType toBits() const { return bits_; }

  constexpr Fixed() : bits_(0) {}

  constexpr BitsType getFractionBits() const { return bits_ & FractionBitMask; }

  template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
  constexpr explicit Fixed(T x)
      : bits_(makeSafe(x) << P) {}

  template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
  constexpr explicit Fixed(T x)
      : bits_(makeSafe(x) * OneBitValue) {}

  template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
  constexpr explicit operator T() const { return assertedCast<T>(bits_ >> P); }

  template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
  constexpr explicit operator T() const { return bits_ * (T(1) / OneBitValue); }

  template<int PR, TEnableIf<(P > PR)>* = nullptr>
  constexpr explicit operator Fixed<PR>() const {
    return Fixed<PR>::fromBits(bits_ >> (P - PR));
  }

  template<int PR, TEnableIf<(P < PR)>* = nullptr>
  constexpr explicit operator Fixed<PR>() const {
    return Fixed<PR>::fromBits(makeSafe(bits_) << (PR - P));
  }

  constexpr explicit operator bool() const { return bits_ != 0; }

  constexpr Fixed operator-() const {
    ASSERT(bits_ != Limits<BitsType>::Min);
    return fromBits(-bits_);
  }

  constexpr Fixed operator+(Fixed rhs) const { return fromBits(makeSafe(bits_) + rhs.bits_); }
  constexpr Fixed operator-(Fixed rhs) const { return fromBits(makeSafe(bits_) - rhs.bits_); }

  constexpr void operator+=(Fixed o) { *this = *this + o; }
  constexpr void operator-=(Fixed o) { *this = *this - o; }

  constexpr Fixed operator<<(int n) const { return fromBits(makeSafe(bits_) << n); }
  constexpr Fixed operator>>(int n) const { return fromBits(bits_ >> n); }

  constexpr void operator<<=(int n) { *this = *this << n; }
  constexpr void operator>>=(int n) { *this = *this >> n; }

  friend constexpr Fixed operator*(Fixed lhs, Fixed rhs) {
    return fromBits(makeSafe(static_cast<WideBitsType>(lhs.bits_) * rhs.bits_) >> P);
  }

  friend constexpr Fixed operator/(Fixed lhs, Fixed rhs) {
    ASSERT(rhs.bits_ != 0);
    return fromBits(makeSafe(static_cast<WideBitsType>(lhs.bits_) << P) / rhs.bits_);
  }

  template<typename T, TEnableIf<TIsArithmetic<T>>* = nullptr>
  constexpr Fixed operator*(T rhs) const { return fromBits(makeSafe(bits_) * rhs); }

  template<typename T, TEnableIf<TIsArithmetic<T>>* = nullptr>
  constexpr Fixed operator/(T rhs) const { return fromBits(makeSafe(bits_) / rhs); }

  template<typename T, TEnableIf<TIsArithmetic<T>>* = nullptr>
  friend constexpr Fixed operator*(T lhs, Fixed rhs) { return rhs * lhs; }

  template<typename T, TEnableIf<TIsArithmetic<T>>* = nullptr>
  friend constexpr Fixed operator/(T lhs, Fixed rhs) { return Fixed(lhs) / rhs; }

  friend constexpr bool operator==(Fixed l, Fixed r) { return l.bits_ == r.bits_; }
  friend constexpr bool operator!=(Fixed l, Fixed r) { return l.bits_ != r.bits_; }
  friend constexpr bool operator<=(Fixed l, Fixed r) { return l.bits_ <= r.bits_; }
  friend constexpr bool operator>=(Fixed l, Fixed r) { return l.bits_ >= r.bits_; }
  friend constexpr bool operator< (Fixed l, Fixed r) { return l.bits_ <  r.bits_; }
  friend constexpr bool operator> (Fixed l, Fixed r) { return l.bits_ >  r.bits_; }

  friend constexpr HashCode partialHash(const Fixed& x) { return static_cast<HashCode>(x.bits_); }

  friend void format(TextWriter& out, const Fixed& x, const StringSpan& opts) {
    detail::fixedFormat(out, opts, x.bits_, P);
  }
  friend TextWriter& operator<<(TextWriter& out, const Fixed& x) {
    detail::fixedFormat(out, x.bits_, P); return out;
  }

  friend Fixed mathAbs(Fixed x) { return x.bits_ >= 0 ? x : -x; }

  friend bool isNear(Fixed x, Fixed y, Fixed tolerance) {
    return mathAbs(x - y) <= tolerance;
  }

  friend constexpr Fixed lerp(Fixed a, Fixed b, double t) {
    return fromBits(lerp(a.bits_, b.bits_, t));
  }
  friend constexpr Fixed lerp(Fixed x, Fixed y, Fixed t) {
    ASSERT(0 <= t.bits_ && t.bits_ <= OneBitValue);
    auto a = static_cast<WideBitsType>(x.bits_) * (OneBitValue - t.bits_);
    auto b = static_cast<WideBitsType>(y.bits_) * t.bits_;
    return fromBits(assertedCast<int32_t>((a + b) >> P));
  }

  friend constexpr Fixed mathFusedMulAdd(Fixed x, Fixed y, Fixed z) {
    auto res64 = static_cast<WideBitsType>(x.bits_) * y.bits_ + z.bits_;
    return fromBits(assertedCast<BitsType>(res64 >> P));
  }

  friend Fixed mathSqrt(Fixed x) { return fromBits(detail::fixedSqrt(x.bits_, P)); }

  friend Fixed mathRsqrt(Fixed x) {
    ASSERT(P == 16);
    return fromBits(detail::fixedRsqrt16(x.bits_));
  }

  friend constexpr int mathFloorToInt(Fixed x) { return x.bits_ >> P; }
  friend constexpr int mathCeilToInt(Fixed x) { return (makeSafe(x.bits_) + FractionBitMask) >> P; }
  friend constexpr int mathTruncToInt(Fixed x) { return x.bits_ >= 0 ? mathFloorToInt(x) : mathCeilToInt(x); }

  friend constexpr int mathRoundToInt(Fixed x) {
    constexpr Fixed half = fromBits(HalfBitValue);
    return x.bits_ >= 0 ? mathFloorToInt(x + half) : mathCeilToInt(x - half);
  }

  friend constexpr Fixed mathFloor(Fixed x) {
    return fromBits(x.bits_ & ~FractionBitMask);
  }
  friend constexpr Fixed mathCeil(Fixed x) {
    return fromBits((makeSafe(x.bits_) + FractionBitMask) & ~FractionBitMask);
  }
  friend constexpr Fixed mathTrunc(Fixed x) {
    return x.bits_ >= 0 ? mathFloor(x) : mathCeil(x);
  }
  friend constexpr Fixed mathRound(Fixed x) {
    constexpr Fixed half = fromBits(HalfBitValue);
    return x.bits_ >= 0 ? mathFloor(x + half) : mathCeil(x - half);
  }

  struct DecomposeResult {
    Fixed integral;
    Fixed fractional;

    void unpack(Fixed& out_integral, Fixed& out_fractional) const {
      out_integral = integral;
      out_fractional = fractional;
    }
  };

  friend constexpr DecomposeResult decompose(Fixed x) {
    Fixed truncated = mathTrunc(x);
    return DecomposeResult { truncated, x - truncated };
  }

 private:
  BitsType bits_;
};

template<int N>
struct Limits<Fixed<N>> {
  static constexpr Fixed<N> Epsilon = Fixed<N>::fromBits(1);

  static constexpr Fixed<N> Max = Fixed<N>::fromBits(INT32_MAX);
  static constexpr Fixed<N> Min = Fixed<N>::fromBits(-INT32_MAX);
};

template<int N>
constexpr Fixed<N> mathNextAfter(Fixed<N> x, Fixed<N> dir) {
  return x < dir ? (x + Limits<Fixed<N>>::Epsilon) :
        (x > dir ? (x - Limits<Fixed<N>>::Epsilon) :
         dir);
}

template<int N>
constexpr Fixed<N> Limits<Fixed<N>>::Epsilon;
template<int N>
constexpr Fixed<N> Limits<Fixed<N>>::Max;
template<int N>
constexpr Fixed<N> Limits<Fixed<N>>::Min;

} // namespace stp

#endif // STP_BASE_MATH_FIXED_H_
