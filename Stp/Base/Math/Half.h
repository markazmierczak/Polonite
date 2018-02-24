// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_HALF_H_
#define STP_BASE_MATH_HALF_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Limits.h"

namespace stp {

// 16-bit floating point value
// Format is 1 bit sign, 5 bits exponent, 10 bits mantissa.
class Half {
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

  static constexpr Half fromBits(BitsType bits) { return Half(bits, FromBitsTag()); }
  constexpr ALWAYS_INLINE BitsType toBits() const { return bits_; }

  constexpr Half() : bits_(0) {}

  constexpr BitsType getSignBit() const { return bits_ & SignBitMask; }
  constexpr BitsType getExponentBits() const { return bits_ & ExponentBitMask; }
  constexpr BitsType getMantissaBits() const { return bits_ & MantissaBitMask; }

  constexpr Half operator-() const { return fromBits(bits_ ^ SignBitMask); }

  explicit Half(int f) = delete;
  BASE_EXPORT explicit Half(float f);
  BASE_EXPORT explicit operator float() const;

  friend constexpr Half mathAbs(Half x) { return fromBits(x.bits_ & ~SignBitMask); }

  friend constexpr bool isNaN(Half x) {
    // It's a NaN if the exponent bits are all ones and the mantissa
    // bits are not entirely zeros.
    return mathAbs(x).toBits() > ExponentBitMask;
  }

  friend constexpr bool isFinite(Half x) { return x.getExponentBits() != ExponentBitMask; }
  friend constexpr bool isInfinity(Half x) { return mathAbs(x).toBits() == ExponentBitMask; }
  friend constexpr bool isNormal(Half x) { return x.getExponentBits() != 0 && isFinite(x); }

  // == and != must be implemented separately to handle -0, +0 case.
  constexpr bool operator==(const Half& rhs) const;
  constexpr bool operator!=(const Half& rhs) const;

  constexpr bool operator< (const Half& rhs) const;
  constexpr bool operator> (const Half& rhs) const;
  constexpr bool operator<=(const Half& rhs) const;
  constexpr bool operator>=(const Half& rhs) const;

  friend HashCode partialHash(const Half& x) { return static_cast<HashCode>(x.bits_); }

  friend void format(TextWriter& out, const Half& x, const StringSpan& opts) {
    formatImpl(out, static_cast<float>(x), opts);
  }

 private:
  BitsType bits_;

  BASE_EXPORT static void formatImpl(TextWriter& out, float x, const StringSpan& opts);
};

template<>
struct Limits<Half> {
  static constexpr int Digits = 11;

  static constexpr int MinExponent = -14;
  static constexpr int MaxExponent = 15;

  static constexpr Half Epsilon = Half::fromBits(0x1400);
  static constexpr Half Infinity = Half::fromBits(Half::ExponentBitMask);
  static constexpr Half NaN = Half::fromBits(0x7FFF);
  static constexpr Half SignalingNaN = Half::fromBits(0x7DFF);

  static constexpr Half SmallestNormal =
      Half::fromBits(Half::SignBitMask >> Half::ExponentBitCount);

  static constexpr Half SmallestSubnormal = Half::fromBits(1);

  static constexpr Half Min = Half::fromBits(~SmallestNormal.toBits());
  static constexpr Half Max = -Min;
};

constexpr bool Half::operator==(const Half& rhs) const {
  const Half& lhs = *this;

  if (lhs.toBits() == rhs.toBits())
    return !isNaN(lhs);

  // Check for -0, +0 case:
  if ((lhs.toBits() | rhs.toBits()) == Half::SignBitMask)
    return true;

  return false;
}

constexpr bool Half::operator!=(const Half& rhs) const {
  const Half& lhs = *this;

  if (lhs.toBits() == rhs.toBits())
    return false;

  // Check for -0, +0 case:
  if ((lhs.toBits() | rhs.toBits()) == Half::SignBitMask)
    return false;

  if (isNaN(lhs) || isNaN(rhs))
    return false;

  return true;
}

#define HALF_COMPARISON(OP) \
  constexpr bool Half::operator OP(const Half& rhs) const { \
  const Half& lhs = *this; \
    return static_cast<int16_t>(lhs.toBits()) OP static_cast<int16_t>(rhs.toBits()) && \
           !(isNaN(lhs) || isNaN(rhs)); \
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

  static constexpr BitsType NaNBitValue = Limits<Half>::NaN.toBits();
  static constexpr BitsType SignalingNaNBitValue = Limits<Half>::SignalingNaN.toBits();
  static constexpr BitsType EpsilonBitValue = Limits<Half>::Epsilon.toBits();
};

} // namespace detail

} // namespace stp

#endif // STP_BASE_MATH_HALF_H_
