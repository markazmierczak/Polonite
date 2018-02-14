// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_FIXED_H_
#define STP_BASE_MATH_FIXED_H_

#include "Base/Math/Safe.h"
#include "Base/Type/FormattableFwd.h"
#include "Base/Type/HashableFwd.h"

namespace stp {
namespace detail {

BASE_EXPORT int32_t SqrtFixed(int32_t value, int count);
BASE_EXPORT int64_t LSqrtFixed(int64_t value, int count);

BASE_EXPORT int32_t RSqrtFixed16(int32_t x);

BASE_EXPORT void FormatFixedPoint(
    TextWriter& writer, const StringSpan& opts, int32_t value, int point);

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

  static constexpr Fixed FromBits(BitsType bits) { return Fixed(bits, FromBitsTag()); }
  ALWAYS_INLINE constexpr BitsType ToBits() const { return bits_; }

  constexpr Fixed() : bits_(0) {}

  constexpr BitsType GetFractionBits() const { return bits_ & FractionBitMask; }

  template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
  constexpr explicit Fixed(T x)
      : bits_(MakeSafe(x) << P) {}

  template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
  constexpr explicit Fixed(T x)
      : bits_(MakeSafe(x) * OneBitValue) {}

  template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
  constexpr explicit operator T() const { return AssertedCast<T>(bits_ >> P); }

  template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
  constexpr explicit operator T() const { return bits_ * (T(1) / OneBitValue); }

  template<int PR, TEnableIf<(P > PR)>* = nullptr>
  constexpr explicit operator Fixed<PR>() const {
    return Fixed<PR>::FromBits(bits_ >> (P - PR));
  }

  template<int PR, TEnableIf<(P < PR)>* = nullptr>
  constexpr explicit operator Fixed<PR>() const {
    return Fixed<PR>::FromBits(MakeSafe(bits_) << (PR - P));
  }

  constexpr explicit operator bool() const { return bits_ != 0; }

  constexpr Fixed operator-() const {
    ASSERT(bits_ != Limits<BitsType>::Min);
    return FromBits(-bits_);
  }

  constexpr Fixed operator+(Fixed rhs) const { return FromBits(MakeSafe(bits_) + rhs.bits_); }
  constexpr Fixed operator-(Fixed rhs) const { return FromBits(MakeSafe(bits_) - rhs.bits_); }

  constexpr void operator+=(Fixed o) { *this = *this + o; }
  constexpr void operator-=(Fixed o) { *this = *this - o; }

  constexpr Fixed operator<<(int n) const { return FromBits(MakeSafe(bits_) << n); }
  constexpr Fixed operator>>(int n) const { return FromBits(bits_ >> n); }

  constexpr void operator<<=(int n) { *this = *this << n; }
  constexpr void operator>>=(int n) { *this = *this >> n; }

  friend constexpr Fixed operator*(Fixed lhs, Fixed rhs) {
    return FromBits(MakeSafe(static_cast<WideBitsType>(lhs.bits_) * rhs.bits_) >> P);
  }

  friend constexpr Fixed operator/(Fixed lhs, Fixed rhs) {
    ASSERT(rhs.bits_ != 0);
    return FromBits(MakeSafe(static_cast<WideBitsType>(lhs.bits_) << P) / rhs.bits_);
  }

  template<typename T, TEnableIf<TIsArithmetic<T>>* = nullptr>
  constexpr Fixed operator*(T rhs) const { return FromBits(MakeSafe(bits_) * rhs); }

  template<typename T, TEnableIf<TIsArithmetic<T>>* = nullptr>
  constexpr Fixed operator/(T rhs) const { return FromBits(MakeSafe(bits_) / rhs); }

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

  friend constexpr HashCode Hash(const Fixed& x) { return static_cast<HashCode>(x.bits_); }

  friend void Format(TextWriter& out, const Fixed& x, const StringSpan& opts) {
    detail::FormatFixedPoint(out, opts, x.bits_, P);
  }

  friend Fixed Abs(Fixed x) { return x.bits_ >= 0 ? x : -x; }

  friend bool IsNear(Fixed x, Fixed y, Fixed tolerance) {
    return Abs(x - y) <= tolerance;
  }

  friend constexpr Fixed Lerp(Fixed a, Fixed b, double t) {
    return FromBits(Lerp(a.bits_, b.bits_, t));
  }
  friend constexpr Fixed Lerp(Fixed x, Fixed y, Fixed t) {
    ASSERT(0 <= t.bits_ && t.bits_ <= OneBitValue);
    auto a = static_cast<WideBitsType>(x.bits_) * (OneBitValue - t.bits_);
    auto b = static_cast<WideBitsType>(y.bits_) * t.bits_;
    return FromBits(AssertedCast<int32_t>((a + b) >> P));
  }

  friend constexpr Fixed FusedMulAdd(Fixed x, Fixed y, Fixed z) {
    auto res64 = static_cast<WideBitsType>(x.bits_) * y.bits_ + z.bits_;
    return FromBits(AssertedCast<BitsType>(res64 >> P));
  }

  friend Fixed Sqrt(Fixed x) { return FromBits(detail::SqrtFixed(x.bits_, P)); }

  friend Fixed RSqrt(Fixed x) {
    ASSERT(P == 16);
    return FromBits(detail::RSqrtFixed16(x.bits_));
  }

  friend constexpr int FloorToInt(Fixed x) { return x.bits_ >> P; }
  friend constexpr int CeilToInt(Fixed x) { return (MakeSafe(x.bits_) + FractionBitMask) >> P; }
  friend constexpr int TruncToInt(Fixed x) { return x.bits_ >= 0 ? FloorToInt(x) : CeilToInt(x); }

  friend constexpr int RoundToInt(Fixed x) {
    constexpr Fixed half = FromBits(HalfBitValue);
    return x.bits_ >= 0 ? FloorToInt(x + half) : CeilToInt(x - half);
  }

  friend constexpr Fixed Floor(Fixed x) {
    return FromBits(x.bits_ & ~FractionBitMask);
  }
  friend constexpr Fixed Ceil(Fixed x) {
    return FromBits((MakeSafe(x.bits_) + FractionBitMask) & ~FractionBitMask);
  }
  friend constexpr Fixed Trunc(Fixed x) {
    return x.bits_ >= 0 ? Floor(x) : Ceil(x);
  }
  friend constexpr Fixed Round(Fixed x) {
    constexpr Fixed half = FromBits(HalfBitValue);
    return x.bits_ >= 0 ? Floor(x + half) : Ceil(x - half);
  }

  struct DecomposeResult {
    Fixed integral;
    Fixed fractional;

    void Unpack(Fixed& out_integral, Fixed& out_fractional) const {
      out_integral = integral;
      out_fractional = fractional;
    }
  };

  friend constexpr DecomposeResult Decompose(Fixed x) {
    Fixed truncated = Trunc(x);
    return DecomposeResult { truncated, x - truncated };
  }

 private:
  BitsType bits_;
};

template<int N>
struct Limits<Fixed<N>> {
  static constexpr Fixed<N> Epsilon = Fixed<N>::FromBits(1);

  static constexpr Fixed<N> Max = Fixed<N>::FromBits(INT32_MAX);
  static constexpr Fixed<N> Min = Fixed<N>::FromBits(-INT32_MAX);
};

template<int N>
constexpr Fixed<N> NextAfter(Fixed<N> x, Fixed<N> dir) {
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
