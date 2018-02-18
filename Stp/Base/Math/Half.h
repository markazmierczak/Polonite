// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_HALF_H_
#define STP_BASE_MATH_HALF_H_

#include "Base/Export.h"
#include "Base/Type/FormattableFwd.h"
#include "Base/Type/Limits.h"

namespace stp {

// 16-bit floating point value
// Format is 1 bit sign, 5 bits exponent, 10 bits mantissa.
class BASE_EXPORT Half {
 public:
  typedef uint16_t BitsType;

  static constexpr int BitCount = 8 * sizeof(BitsType);

  static constexpr int MantissaBitCount = 10;

  static constexpr int ExponentBitCount = BitCount - 1 - MantissaBitCount;

  static constexpr BitsType SignBitMask = static_cast<BitsType>(1) << (BitCount - 1);

  static constexpr BitsType MantissaBitMask =
      static_cast<BitsType>(~static_cast<BitsType>(0)) >> (ExponentBitCount + 1);

  static constexpr BitsType ExponentBitMask = static_cast<BitsType>(~(SignBitMask | MantissaBitMask));

 private:
  struct FromBitsTag {};
  constexpr Half(BitsType bits, FromBitsTag) : bits_(bits) {}
 public:

  static constexpr Half FromBits(BitsType bits) { return Half(bits, FromBitsTag()); }
  constexpr ALWAYS_INLINE BitsType ToBits() const { return bits_; }

  constexpr Half() : bits_(0) {}

  constexpr BitsType GetSignBit() const { return bits_ & SignBitMask; }

  constexpr BitsType GetExponentBits() const { return bits_ & ExponentBitMask; }

  constexpr BitsType GetMantissaBits() const { return bits_ & MantissaBitMask; }

  constexpr Half operator-() const { return FromBits(bits_ ^ SignBitMask); }

  explicit Half(float f);
  explicit Half(int f) = delete;

  explicit operator float() const;

  friend constexpr Half Abs(Half x) { return FromBits(x.bits_ & ~SignBitMask); }

  friend constexpr bool IsNaN(Half x) {
    // It's a NaN if the exponent bits are all ones and the mantissa
    // bits are not entirely zeros.
    return Abs(x).ToBits() > ExponentBitMask;
  }

  friend constexpr bool IsFinite(Half x) { return x.GetExponentBits() != ExponentBitMask; }

  friend constexpr bool IsInfinity(Half x) { return Abs(x).ToBits() == ExponentBitMask; }

  friend constexpr bool IsNormal(Half x) { return x.GetExponentBits() != 0 && IsFinite(x); }

  // == and != must be implemented separately to handle -0, +0 case.
  constexpr bool operator==(const Half& rhs) const;
  constexpr bool operator!=(const Half& rhs) const;

  constexpr bool operator< (const Half& rhs) const;
  constexpr bool operator> (const Half& rhs) const;
  constexpr bool operator<=(const Half& rhs) const;
  constexpr bool operator>=(const Half& rhs) const;

  friend HashCode Hash(const Half& x) { return static_cast<HashCode>(x.bits_); }

  friend void Format(TextWriter& out, const Half& x, const StringSpan& opts) {
    FormatImpl(out, static_cast<float>(x), opts);
  }

 private:
  BitsType bits_;

  static void FormatImpl(TextWriter& out, float x, const StringSpan& opts);
};

template<>
struct BASE_EXPORT Limits<Half> {
  static constexpr int Digits = 11;

  static constexpr int MinExponent = -14;
  static constexpr int MaxExponent = 15;

  static constexpr Half Epsilon = Half::FromBits(0x1400);
  static constexpr Half Infinity = Half::FromBits(Half::ExponentBitMask);
  static constexpr Half NaN = Half::FromBits(0x7FFF);
  static constexpr Half SignalingNaN = Half::FromBits(0x7DFF);

  static constexpr Half SmallestNormal =
      Half::FromBits(Half::SignBitMask >> Half::ExponentBitCount);

  static constexpr Half SmallestSubnormal = Half::FromBits(1);

  static constexpr Half Min = Half::FromBits(~SmallestNormal.ToBits());
  static constexpr Half Max = -Min;
};

constexpr bool Half::operator==(const Half& rhs) const {
  const Half& lhs = *this;

  if (lhs.ToBits() == rhs.ToBits())
    return !IsNaN(lhs);

  // Check for -0, +0 case:
  if ((lhs.ToBits() | rhs.ToBits()) == Half::SignBitMask)
    return true;

  return false;
}

constexpr bool Half::operator!=(const Half& rhs) const {
  const Half& lhs = *this;

  if (lhs.ToBits() == rhs.ToBits())
    return false;

  // Check for -0, +0 case:
  if ((lhs.ToBits() | rhs.ToBits()) == Half::SignBitMask)
    return false;

  if (IsNaN(lhs) || IsNaN(rhs))
    return false;

  return true;
}

#define HALF_COMPARISON(OP) \
  constexpr bool Half::operator OP(const Half& rhs) const { \
  const Half& lhs = *this; \
    return static_cast<int16_t>(lhs.ToBits()) OP static_cast<int16_t>(rhs.ToBits()) && \
           !(IsNaN(lhs) || IsNaN(rhs)); \
  }

HALF_COMPARISON(<)
HALF_COMPARISON(>)
HALF_COMPARISON(<=)
HALF_COMPARISON(>=)

#undef HALF_COMPARISON

namespace detail {

template<typename T>
struct RawFloatingPointTraits;

template<>
struct RawFloatingPointTraits<Half> {
  typedef uint16_t BitsType;

  static constexpr BitsType NaNBitValue = Limits<Half>::NaN.ToBits();
  static constexpr BitsType SignalingNaNBitValue = Limits<Half>::SignalingNaN.ToBits();
  static constexpr BitsType EpsilonBitValue = Limits<Half>::Epsilon.ToBits();
};

} // namespace detail

} // namespace stp

#endif // STP_BASE_MATH_HALF_H_
