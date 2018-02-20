// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2005, Google Inc.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Based on parts from Google C++ Testing Framework.

#ifndef STP_BASE_MATH_RAWFLOAT_H_
#define STP_BASE_MATH_RAWFLOAT_H_

#include "Base/Type/Limits.h"

namespace stp {

namespace detail {

template<typename T>
struct RawFloatingPointTraits;

template<>
struct RawFloatingPointTraits<float> {
  typedef uint32_t BitsType;

  static const BitsType EpsilonBitValue = UINT32_C(0x34000000);
  static const BitsType NaNBitValue = UINT32_C(0x7FC00000);
};

template<>
struct RawFloatingPointTraits<double> {
  typedef uint64_t BitsType;

  static const BitsType EpsilonBitValue = UINT64_C(0x3CB0000000000000);
  static const BitsType NaNBitValue = UINT64_C(0x7FF8000000000000);
};

} // namespace detail

// This template class represents an IEEE floating-point number
// (either single-precision or double-precision, depending on the
// template parameters).
//
// The purpose of this class is to do more sophisticated number comparison.
// (Due to round-off error, etc, it's very unlikely that
// two floating-points will be equal exactly. Hence a naive
// comparison by the == operation often doesn't work.)
//
// Format of IEEE floating-point:
//
//   The most-significant bit being the leftmost, an IEEE
//   floating-point looks like
//
//     sign_bit exponent_bits mantissa_bits
//
//   Here, sign_bit is a single bit that designates the sign of the number.
//
//   For float, there are 8 exponent bits and 23 mantissa bits.
//
//   For double, there are 11 exponent bits and 52 mantissa bits.
//
//   More details can be found at
//   http://en.wikipedia.org/wiki/IEEE_floating-point_standard.
//
// Template parameter:
//
//  TBuiltin: the floating-point type (either float or double)
template<typename TBuiltin>
class RawFloatingPoint {
 public:
  using Traits = typename detail::RawFloatingPointTraits<TBuiltin>;

  // Defines the unsigned integer type that has the same size as the
  // floating point number.
  typedef typename Traits::BitsType BitsType;

 private:
  struct FromBitsCtor {};
  constexpr RawFloatingPoint(BitsType bits, FromBitsCtor) : bits_(bits) {}
 public:

  static constexpr RawFloatingPoint FromBits(BitsType bits) {
    return RawFloatingPoint(bits, FromBitsCtor());
  }
  constexpr ALWAYS_INLINE BitsType ToBits() const { return bits_; }

  static constexpr int BitCount = 8 * sizeof(BitsType);

  static constexpr int MantissaBitCount = Limits<TBuiltin>::Digits - 1;

  static constexpr int ExponentBitCount = BitCount - 1 - MantissaBitCount;

  static constexpr BitsType SignBitMask = static_cast<BitsType>(1) << (BitCount - 1);

  static constexpr BitsType MantissaBitMask = ~static_cast<BitsType>(0) >> (ExponentBitCount + 1);

  static constexpr BitsType ExponentBitMask = ~(SignBitMask | MantissaBitMask);

  // How many ULP's (Units in the Last Place) we want to tolerate when
  // comparing two numbers. The larger the value, the more error we
  // allow. A 0 value means that two numbers must be exactly the same
  // to be considered equal.
  //
  // The maximum error of a single floating-point operation is 0.5
  // units in the last place. On Intel CPU's, all floating-point
  // calculations are done with 80-bit precision, while double has 64
  // bits. Therefore, 4 should be enough for ordinary use.
  //
  // See the following article for more details on ULP:
  // http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
  static constexpr int MaxUlps = 4;

  constexpr RawFloatingPoint() : bits_(0) {}

  // Constructs a BasicFloat from an unpacked floating-point number.
  //
  // On an Intel CPU, passing a non-normalized NaN
  // around may change its bits, although the new value is guaranteed
  // to be also a NaN. Therefore, don't expect this constructor to
  // preserve the bits in x when x is a NaN.
  explicit RawFloatingPoint(TBuiltin x) {
    U u;
    u.value = x;
    bits_ = u.bits;
  }

  // Reinterprets a bit pattern as a floating-point number.
  explicit operator TBuiltin() const {
    U u;
    u.bits = bits_;
    return u.value;
  }

  constexpr BitsType GetSignBit() const { return bits_ & SignBitMask; }

  constexpr BitsType GetExponentBits() const { return bits_ & ExponentBitMask; }

  constexpr BitsType GetMantissaBits() const { return bits_ & MantissaBitMask; }

  constexpr RawFloatingPoint operator-() const { return FromBits(bits_ ^ SignBitMask); }

  friend constexpr RawFloatingPoint Abs(RawFloatingPoint x) {
    return FromBits(x.bits_ & ~SignBitMask);
  }

  friend constexpr bool IsNan(RawFloatingPoint x) {
    // It's a NaN if the exponent bits are all ones and the mantissa
    // bits are not entirely zeros.
    return Abs(x).ToBits() > ExponentBitMask;
  }

  friend constexpr bool isFinite(RawFloatingPoint x) {
    return x.GetExponentBits() != ExponentBitMask;
  }

  friend constexpr bool isInfinity(RawFloatingPoint x) {
    return Abs(x).ToBits() == ExponentBitMask;
  }

  friend constexpr bool IsNormal(RawFloatingPoint x) {
    return x.GetExponentBits() != 0 && isFinite(x);
  }

  // Returns true iff this number is at most MaxUlps ULP's away from rhs.
  // In particular, this function:
  //   - returns false if either number is (or both are) NaN.
  //   - treats really large numbers as almost equal to infinity.
  //   - thinks +0.0 and -0.0 are 0 DLP's apart.
  friend constexpr bool isNearUlp(const RawFloatingPoint& lhs, const RawFloatingPoint& rhs) {
    // The IEEE standard says that any comparison operation involving
    // a NaN must return false.
    if (IsNan(lhs) || IsNan(rhs))
      return false;

    return DistanceBetweenSignAndMagnitudeNumbers(lhs.bits_, rhs.bits_) <= MaxUlps;
  }

 private:
  BitsType bits_;

  // The data type used to store the actual floating-point number.
  union U {
    TBuiltin value; // The unpacked floating-point number.
    BitsType bits;  // The bits that represent the number.
  };

  // Converts an integer from the sign-and-magnitude representation to
  // the biased representation. More precisely, let N be 2 to the
  // power of (BitCount - 1), an integer x is represented by the
  // unsigned number x + N.
  //
  // For instance,
  //
  //   -N + 1 (the most negative number representable using
  //          sign-and-magnitude) is represented by 1;
  //   0      is represented by N; and
  //   N - 1  (the biggest number representable using
  //          sign-and-magnitude) is represented by 2N - 1.
  //
  // Read http://en.wikipedia.org/wiki/Signed_number_representations
  // for more details on signed number representations.
  static constexpr BitsType SignAndMagnitudeToBiased(BitsType sam) {
    if (SignBitMask & sam) {
      // sam represents a negative number.
      return ~sam + 1;
    }
    // sam represents a positive number.
    return SignBitMask | sam;
  }

  // Given two numbers in the sign-and-magnitude representation,
  // returns the distance between them as an unsigned number.
  static constexpr BitsType DistanceBetweenSignAndMagnitudeNumbers(BitsType sam1, BitsType sam2) {
    const BitsType biased1 = SignAndMagnitudeToBiased(sam1);
    const BitsType biased2 = SignAndMagnitudeToBiased(sam2);
    return (biased1 >= biased2) ? (biased1 - biased2) : (biased2 - biased1);
  }
};

using RawFloat = RawFloatingPoint<float>;
using RawDouble = RawFloatingPoint<double>;

template<typename T>
struct Limits<RawFloatingPoint<T>> {
  typedef RawFloatingPoint<T> RawType;
  typedef typename detail::RawFloatingPointTraits<T> Traits;

  static constexpr int Digits = Limits<T>::Digits;

  static constexpr int MinExponent = Limits<T>::MinExponent;
  static constexpr int MaxExponent = Limits<T>::MaxExponent;

  static constexpr RawType Epsilon = RawType::FromBits(Traits::EpsilonBitValue);
  static constexpr RawType Infinity = RawType::FromBits(RawType::ExponentBitMask);
  static constexpr RawType NaN = RawType::FromBits(Traits::NaNBitValue);

  static constexpr RawType SmallestNormal =
      RawType::FromBits(RawType::SignBitMask >> RawType::ExponentBitCount);

  static constexpr RawType SmallestSubnormal = RawType::FromBits(1);

  static constexpr RawType Min = RawType::FromBits(~SmallestNormal.ToBits());
  static constexpr RawType Max = -Min;
};

template<typename T>
constexpr RawFloatingPoint<T> Limits<RawFloatingPoint<T>>::Epsilon;
template<typename T>
constexpr RawFloatingPoint<T> Limits<RawFloatingPoint<T>>::Infinity;
template<typename T>
constexpr RawFloatingPoint<T> Limits<RawFloatingPoint<T>>::NaN;
template<typename T>
constexpr RawFloatingPoint<T> Limits<RawFloatingPoint<T>>::SmallestNormal;
template<typename T>
constexpr RawFloatingPoint<T> Limits<RawFloatingPoint<T>>::SmallestSubnormal;
template<typename T>
constexpr RawFloatingPoint<T> Limits<RawFloatingPoint<T>>::Max;
template<typename T>
constexpr RawFloatingPoint<T> Limits<RawFloatingPoint<T>>::Min;

} // namespace stp

#endif // STP_BASE_MATH_RAWFLOAT_H_
