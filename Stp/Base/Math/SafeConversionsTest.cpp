// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Math/SafeConversions.h"

#include "Base/Compiler/Cpu.h"
#include "Base/Compiler/Os.h"
#include "Base/Math/Math.h"
#include "Base/Test/GTest.h"

#if COMPILER(MSVC) && CPU(32BIT)
#include <mmintrin.h>
#endif

// These tests deliberately cause arithmetic overflows. If the compiler is
// aggressive enough, it can const fold these overflows. Disable warnings about
// overflows for const expressions.
#if OS(WIN)
#pragma warning(disable:4756)
#endif

namespace stp {

// This is a helper function for finding the maximum value in Src that can be
// wholy represented as the destination floating-point type.
template<typename Dst, typename Src>
Dst GetMaxConvertibleToFloat() {
  typedef Limits<Dst> DstLimits;
  typedef Limits<Src> SrcLimits;
  static_assert(TIsFloatingPoint<Dst>, "!");

  if (SrcLimits::Digits <= DstLimits::Digits &&
      detail::TNumericMaxExponent<Src>::Value <= detail::TNumericMaxExponent<Dst>::Value)
    return SrcLimits::Max;
  Src max = SrcLimits::Max / 2 + (SrcLimits::is_integer ? 1 : 0);
  while (max != static_cast<Src>(static_cast<Dst>(max))) {
    max /= 2;
  }
  return static_cast<Dst>(max);
}

// Enumerates the five different conversions types we need to test.
enum NumericConversionType {
  SIGN_PRESERVING_VALUE_PRESERVING,
  SIGN_PRESERVING_NARROW,
  SIGN_TO_UNSIGN_WIDEN_OR_EQUAL,
  SIGN_TO_UNSIGN_NARROW,
  UNSIGN_TO_SIGN_NARROW_OR_EQUAL,
};

// Template covering the different conversion tests.
template<typename Dst, typename Src, NumericConversionType conversion>
struct TestNumericConversion {};

// EXPECT_EQ wrappers providing specific detail on test failures.
#define TEST_EXPECTED_RANGE(expected, actual)                                  \
  EXPECT_EQ(expected, detail::DstRangeRelationToSrcRange<Dst>(actual)) \
      << "Conversion test: " << src << " value " << actual << " to " << dst    \
      << " on line " << line;

TEST(SafeNumerics, CastTests) {
  // MSVC catches and warns that we're forcing saturation in these tests.
  // Since that's intentional, we need to shut this warning off.
  #if COMPILER(MSVC)
  #pragma warning(disable : 4756)
  #endif

  int small_positive = 1;
  int small_negative = -1;
  double double_small = 1.0;
  double double_large = Limits<double>::Max;
  double double_infinity = Limits<float>::Infinity;
  double double_large_int = Limits<int>::Max;
  double double_small_int = Limits<int>::Min;

  // Just test that the casts compile, since the other tests cover logic.
  EXPECT_EQ(0, AssertedCast<int>(static_cast<size_t>(0)));
  EXPECT_EQ(0, StrictCast<int>(static_cast<unsigned char>(0)));
  EXPECT_EQ(0U, StrictCast<unsigned>(static_cast<unsigned char>(0)));

  // These casts and coercions will fail to compile:
  // EXPECT_EQ(0, StrictCast<int>(static_cast<size_t>(0)));
  // EXPECT_EQ(0, StrictCast<size_t>(static_cast<int>(0)));
  // EXPECT_EQ(1ULL, StrictNumeric<size_t>(1));
  // EXPECT_EQ(1, StrictNumeric<size_t>(1U));

  // Test various saturation corner cases.
  EXPECT_EQ(SaturatedCast<int>(small_negative), static_cast<int>(small_negative));
  EXPECT_EQ(SaturatedCast<int>(small_positive), static_cast<int>(small_positive));
  EXPECT_EQ(SaturatedCast<unsigned>(small_negative), static_cast<unsigned>(0));
  EXPECT_EQ(SaturatedCast<int>(double_small), static_cast<int>(double_small));
  EXPECT_EQ(SaturatedCast<int>(double_large), Limits<int>::Max);
  EXPECT_EQ(SaturatedCast<float>(double_large), double_infinity);
  EXPECT_EQ(SaturatedCast<float>(-double_large), -double_infinity);
  EXPECT_EQ(Limits<int>::Min, SaturatedCast<int>(double_small_int));
  EXPECT_EQ(Limits<int>::Max, SaturatedCast<int>(double_large_int));

  float not_a_number = Limits<float>::Infinity - Limits<float>::Infinity;
  EXPECT_TRUE(isNaN(not_a_number));
  EXPECT_EQ(0, SaturatedCast<int>(not_a_number));
}

TEST(SafeNumerics, IsValueInRangeForNumericType) {
  EXPECT_TRUE(IsValueInRangeForNumericType<uint32_t>(0));
  EXPECT_TRUE(IsValueInRangeForNumericType<uint32_t>(1));
  EXPECT_TRUE(IsValueInRangeForNumericType<uint32_t>(2));
  EXPECT_FALSE(IsValueInRangeForNumericType<uint32_t>(-1));
  EXPECT_TRUE(IsValueInRangeForNumericType<uint32_t>(0xffffffffu));
  EXPECT_TRUE(IsValueInRangeForNumericType<uint32_t>(UINT64_C(0xffffffff)));
  EXPECT_FALSE(IsValueInRangeForNumericType<uint32_t>(UINT64_C(0x100000000)));
  EXPECT_FALSE(IsValueInRangeForNumericType<uint32_t>(UINT64_C(0x100000001)));
  EXPECT_FALSE(IsValueInRangeForNumericType<uint32_t>(Limits<int32_t>::Min));
  EXPECT_FALSE(IsValueInRangeForNumericType<uint32_t>(Limits<int64_t>::Min));

  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(0));
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(1));
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(2));
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(-1));
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(0x7fffffff));
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(0x7fffffffu));
  EXPECT_FALSE(IsValueInRangeForNumericType<int32_t>(0x80000000u));
  EXPECT_FALSE(IsValueInRangeForNumericType<int32_t>(0xffffffffu));
  EXPECT_FALSE(IsValueInRangeForNumericType<int32_t>(INT64_C(0x80000000)));
  EXPECT_FALSE(IsValueInRangeForNumericType<int32_t>(INT64_C(0xffffffff)));
  EXPECT_FALSE(IsValueInRangeForNumericType<int32_t>(INT64_C(0x100000000)));
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(Limits<int32_t>::Min));
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(
      static_cast<int64_t>(Limits<int32_t>::Min)));
  EXPECT_FALSE(IsValueInRangeForNumericType<int32_t>(
      static_cast<int64_t>(Limits<int32_t>::Min) - 1));
  EXPECT_FALSE(IsValueInRangeForNumericType<int32_t>(Limits<int64_t>::Min));

  EXPECT_TRUE(IsValueInRangeForNumericType<uint64_t>(0));
  EXPECT_TRUE(IsValueInRangeForNumericType<uint64_t>(1));
  EXPECT_TRUE(IsValueInRangeForNumericType<uint64_t>(2));
  EXPECT_FALSE(IsValueInRangeForNumericType<uint64_t>(-1));
  EXPECT_TRUE(IsValueInRangeForNumericType<uint64_t>(0xffffffffu));
  EXPECT_TRUE(IsValueInRangeForNumericType<uint64_t>(UINT64_C(0xffffffff)));
  EXPECT_TRUE(IsValueInRangeForNumericType<uint64_t>(UINT64_C(0x100000000)));
  EXPECT_TRUE(IsValueInRangeForNumericType<uint64_t>(UINT64_C(0x100000001)));
  EXPECT_FALSE(IsValueInRangeForNumericType<uint64_t>(Limits<int32_t>::Min));
  EXPECT_FALSE(IsValueInRangeForNumericType<uint64_t>(INT64_C(-1)));
  EXPECT_FALSE(IsValueInRangeForNumericType<uint64_t>(Limits<int64_t>::Min));

  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(0));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(1));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(2));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(-1));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(0x7fffffff));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(0x7fffffffu));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(0x80000000u));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(0xffffffffu));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(INT64_C(0x80000000)));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(INT64_C(0xffffffff)));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(INT64_C(0x100000000)));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(INT64_C(0x7fffffffffffffff)));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(UINT64_C(0x7fffffffffffffff)));
  EXPECT_FALSE(IsValueInRangeForNumericType<int64_t>(UINT64_C(0x8000000000000000)));
  EXPECT_FALSE(IsValueInRangeForNumericType<int64_t>(UINT64_C(0xffffffffffffffff)));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(Limits<int32_t>::Min));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(
      static_cast<int64_t>(Limits<int32_t>::Min)));
  EXPECT_TRUE(IsValueInRangeForNumericType<int64_t>(Limits<int64_t>::Min));
}

} // namespace stp
