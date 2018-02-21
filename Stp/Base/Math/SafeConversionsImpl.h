// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_MATH_SAFECONVERSIONSIMPL_H_
#define STP_BASE_MATH_SAFECONVERSIONSIMPL_H_

#include "Base/Math/Math.h"
#include "Base/Type/Limits.h"

namespace stp {
namespace detail {

// The std library doesn't provide a binary max_exponent for integers, however
// we can compute an analog using Limits<>::Digits.
template<typename T, typename TEnabler = void>
struct TNumericMaxExponent;

template<typename T>
struct TNumericMaxExponent<T, TEnableIf<TIsInteger<T>>> {
  static constexpr int Value = Limits<T>::Digits;
};

template<typename T>
struct TNumericMaxExponent<T, TEnableIf<TIsFloatingPoint<T>>> {
  static constexpr int Value = Limits<T>::MaxExponent;
};

// This performs a fast negation, returning a signed value. It works on unsigned
// arguments, but probably doesn't do what you want for any unsigned value
// larger than max / 2 + 1 (i.e. signed min cast to unsigned).
template<typename T>
constexpr TMakeSigned<T> conditionalNegate(T x, bool is_negative) {
  static_assert(TIsInteger<T>, "type must be integral");
  using SignedT = TMakeSigned<T>;
  using UnsignedT = TMakeUnsigned<T>;
  return static_cast<SignedT>((static_cast<UnsignedT>(x) ^ -SignedT(is_negative)) + is_negative);
}

enum IntegerRepresentation {
  IntegerRepresentationUnsigned,
  IntegerRepresentationSigned
};

// A range for a given nunmeric Src type is contained for a given numeric Dst
// type if both Limits<Src>::Max <= Limits<Dst>::Max and
// Limits<Src>::Min >= Limits<Dst>::Min are true.
// We implement this as template specializations rather than simple static
// comparisons to ensure type correctness in our comparisons.
enum NumericRangeRepresentation {
  NumericRangeNotContained,
  NumericRangeContained
};

// Helper templates to statically determine if our destination type can contain
// maximum and minimum values represented by the source type.

template<typename TDst,
         typename TSrc,
         IntegerRepresentation DstSign = TIsSigned<TDst>
             ? IntegerRepresentationSigned : IntegerRepresentationUnsigned,
         IntegerRepresentation SrcSign = TIsSigned<TSrc>
             ? IntegerRepresentationSigned : IntegerRepresentationUnsigned>
struct StaticDstRangeRelationToSrcRange;

// Same sign: Dst is guaranteed to contain Src only if its range is equal or larger.
template<typename TDst, typename TSrc, IntegerRepresentation Sign>
struct StaticDstRangeRelationToSrcRange<TDst, TSrc, Sign, Sign> {
  static constexpr NumericRangeRepresentation Value =
      TNumericMaxExponent<TDst>::Value >= TNumericMaxExponent<TSrc>::Value
          ? NumericRangeContained
          : NumericRangeNotContained;
};

// Unsigned to signed: Dst is guaranteed to contain source only if its range is
// larger.
template<typename TDst, typename TSrc>
struct StaticDstRangeRelationToSrcRange<
    TDst, TSrc,
    IntegerRepresentationSigned, IntegerRepresentationUnsigned> {
  static constexpr NumericRangeRepresentation Value =
      TNumericMaxExponent<TDst>::Value > TNumericMaxExponent<TSrc>::Value
          ? NumericRangeContained
          : NumericRangeNotContained;
};

// Signed to unsigned: Dst cannot be statically determined to contain Src.
template<typename TDst, typename TSrc>
struct StaticDstRangeRelationToSrcRange<
    TDst, TSrc,
    IntegerRepresentationUnsigned, IntegerRepresentationSigned> {
  static const NumericRangeRepresentation Value = NumericRangeNotContained;
};

// This class wraps the range constraints as separate booleans so the compiler
// can identify constants and eliminate unused code paths.
class RangeCheck {
 public:
  constexpr RangeCheck(bool is_in_lower_bound, bool is_in_upper_bound)
      : is_underflow_(!is_in_lower_bound), is_overflow_(!is_in_upper_bound) {}

  constexpr RangeCheck() : is_underflow_(0), is_overflow_(0) {}

  constexpr bool isValid() const { return !is_overflow_ && !is_underflow_; }
  constexpr bool isInvalid() const { return is_overflow_ && is_underflow_; }
  constexpr bool isOverflow() const { return is_overflow_ && !is_underflow_; }
  constexpr bool isUnderflow() const { return !is_overflow_ && is_underflow_; }

  constexpr bool isOverflowFlagSet() const { return is_overflow_; }
  constexpr bool isUnderflowFlagSet() const { return is_underflow_; }

  constexpr bool operator==(const RangeCheck rhs) const {
    return is_underflow_ == rhs.is_underflow_ && is_overflow_ == rhs.is_overflow_;
  }
  constexpr bool operator!=(const RangeCheck rhs) const {
    return !(*this == rhs);
  }

 private:
  // Do not change the order of these member variables. The integral conversion
  // optimization depends on this exact order.
  const bool is_underflow_;
  const bool is_overflow_;
};

// The following helper template addresses a corner case in range checks for
// conversion from a floating-point type to an integral type of smaller range
// but larger precision (e.g. float -> unsigned). The problem is as follows:
//   1. Integral maximum is always one less than a power of two, so it must be
//      truncated to fit the mantissa of the floating point. The direction of
//      rounding is implementation defined, but by default it's always IEEE
//      floats, which round to nearest and thus result in a value of larger
//      magnitude than the integral value.
//      Example: float f = UINT_MAX; // f is 4294967296f but UINT_MAX
//                                   // is 4294967295u.
//   2. If the floating point value is equal to the promoted integral maximum
//      value, a range check will erroneously pass.
//      Example: (4294967296f <= 4294967295u) // This is true due to a precision
//                                            // loss in rounding up to float.
//   3. When the floating point value is then converted to an integer, the
//      resulting value is out of range for the target integral type and
//      thus is implementation defined.
//      Example: unsigned u = (float)INT_MAX; // u will typically overflow to 0.
// To fix this bug we manually truncate the maximum value when the destination
// type is an integer of larger precision than the source floating-point type,
// such that the resulting maximum is represented exactly as a floating point.
template<typename TDst, typename TSrc>
struct NarrowingRange {
  using SrcLimits = Limits<TSrc>;
  using DstLimits = Limits<TDst>;

  // Computes the mask required to make an accurate comparison between types.
  static const int Shift =
      (TNumericMaxExponent<TSrc>::Value > TNumericMaxExponent<TDst>::Value &&
       SrcLimits::Digits < DstLimits::Digits)
          ? (DstLimits::Digits - SrcLimits::Digits)
          : 0;

  // Masks out the integer bits that are beyond the precision of the
  // intermediate type used for comparison.
  template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
  static constexpr T adjust(T value) {
    static_assert(TsAreSame<T, TDst>, "!");
    static_assert(Shift < DstLimits::Digits, "!");
    return static_cast<T>(conditionalNegate(
        mathAbsToUnsigned(value) & ~((T(1) << Shift) - T(1)),
        isNegative(value)));
  }

  template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
  static constexpr T adjust(T value) {
    static_assert(TsAreSame<T, TDst>, "!");
    static_assert(Shift == 0, "!");
    return value;
  }

  static constexpr TDst Max = adjust(Limits<TDst>::Max);
  static constexpr TDst Min = adjust(Limits<TDst>::Min);
};

template<typename TDst, typename TSrc,
         IntegerRepresentation DstSign = TIsSigned<TDst>
             ? IntegerRepresentationSigned : IntegerRepresentationUnsigned,
         IntegerRepresentation SrcSign = TIsSigned<TSrc>
             ? IntegerRepresentationSigned : IntegerRepresentationUnsigned,
         NumericRangeRepresentation DstRange =
             StaticDstRangeRelationToSrcRange<TDst, TSrc>::Value>
struct DstRangeRelationToSrcRangeImpl;

// The following templates are for ranges that must be verified at runtime. We
// split it into checks based on signedness to avoid confusing casts and
// compiler warnings on signed an unsigned comparisons.

// Same sign narrowing: The range is contained for normal limits.
template<typename TDst, typename TSrc,
         IntegerRepresentation DstSign, IntegerRepresentation SrcSign>
struct DstRangeRelationToSrcRangeImpl<
    TDst, TSrc,
    DstSign, SrcSign,
    NumericRangeContained> {
  static constexpr RangeCheck check(TSrc value) {
    using SrcLimits = Limits<TSrc>;
    using DstLimits = NarrowingRange<TDst, TSrc>;
    return RangeCheck(
        static_cast<TDst>(SrcLimits::Min) >= DstLimits::Min ||
            static_cast<TDst>(value) >= DstLimits::Min,
        static_cast<TDst>(SrcLimits::Max) <= DstLimits::Max ||
            static_cast<TDst>(value) <= DstLimits::Max);
  }
};

// Signed to signed narrowing: Both the upper and lower boundaries may be
// exceeded for standard limits.
template<typename TDst, typename TSrc>
struct DstRangeRelationToSrcRangeImpl<
    TDst, TSrc,
    IntegerRepresentationSigned, IntegerRepresentationSigned,
    NumericRangeNotContained> {
  static constexpr RangeCheck check(TSrc value) {
    using DstLimits = NarrowingRange<TDst, TSrc>;
    return RangeCheck(value >= DstLimits::Min, value <= DstLimits::Max);
  }
};

// Unsigned to unsigned narrowing: Only the upper bound can be exceeded for
// standard limits.
template<typename TDst, typename TSrc>
struct DstRangeRelationToSrcRangeImpl<
    TDst, TSrc,
    IntegerRepresentationUnsigned, IntegerRepresentationUnsigned,
    NumericRangeNotContained> {
  static constexpr RangeCheck check(TSrc value) {
    using DstLimits = NarrowingRange<TDst, TSrc>;
    return RangeCheck(
        DstLimits::Min == TDst(0) || value >= DstLimits::Min,
        value <= DstLimits::Max);
  }
};

// Unsigned to signed: Only the upper bound can be exceeded for standard limits.
template<typename TDst, typename TSrc>
struct DstRangeRelationToSrcRangeImpl<
    TDst, TSrc,
    IntegerRepresentationSigned, IntegerRepresentationUnsigned,
    NumericRangeNotContained> {
  static constexpr RangeCheck check(TSrc value) {
    using DstLimits = NarrowingRange<TDst, TSrc>;
    using Promotion = decltype(TSrc() + TDst());
    return RangeCheck(
        DstLimits::Min <= TDst(0) ||
            static_cast<Promotion>(value) >= static_cast<Promotion>(DstLimits::Min),
        static_cast<Promotion>(value) <= static_cast<Promotion>(DstLimits::Max));
  }
};

// Signed to unsigned: The upper boundary may be exceeded for a narrower Dst,
// and any negative value exceeds the lower boundary for standard limits.
template<typename TDst, typename TSrc>
struct DstRangeRelationToSrcRangeImpl<
    TDst, TSrc,
    IntegerRepresentationUnsigned, IntegerRepresentationSigned,
    NumericRangeNotContained> {
  static constexpr RangeCheck check(TSrc value) {
    using SrcLimits = Limits<TSrc>;
    using DstLimits = NarrowingRange<TDst, TSrc>;
    using Promotion = decltype(TSrc() + TDst());
    return RangeCheck(
        value >= TSrc(0) && (DstLimits::Min == 0 || static_cast<TDst>(value) >= DstLimits::Min),
        static_cast<Promotion>(SrcLimits::Max) <= static_cast<Promotion>(DstLimits::Max) ||
            static_cast<Promotion>(value) <= static_cast<Promotion>(DstLimits::Max));
  }
};

template<typename TDst, typename TSrc>
constexpr RangeCheck dstRangeRelationToSrcRange(TSrc value) {
  static_assert(TIsArithmetic<TSrc>, "argument must be numeric");
  static_assert(TIsArithmetic<TDst>, "result must be numeric");
  return DstRangeRelationToSrcRangeImpl<TDst, TSrc>::check(value);
}

} // namespace detail
} // namespace stp

#endif // STP_BASE_MATH_SAFECONVERSIONSIMPL_H_
