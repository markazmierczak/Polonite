// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/Safe.h"

#include "Base/Test/GTest.h"

namespace stp {

#define EXPECT_VALUE_OF_TYPE(VALUE, TYPE) \
  static_assert(TsAreSame<decltype(VALUE), TYPE>, "!")

TEST(SafeTest, Basic) {
  // Check if we are on machine using 2's complement integers.
  static_assert(static_cast<int>(0xFFFFFFFEu) == -2, "!");

  auto x = MakeSafe(3u);
  EXPECT_VALUE_OF_TYPE(x, Safe<unsigned int>);
  EXPECT_EQ(x.get(), 3u);

  EXPECT_FALSE(!x);
  EXPECT_TRUE(!MakeSafe(0));

  Safe<int8_t> y = -5;
  EXPECT_EQ(y.get(), -5);
}

TEST(SafeTest, MakeSafeNested) {
  auto x = MakeSafe(3);
  auto y = MakeSafe(x);
  EXPECT_EQ(y.get(), 3);
}

TEST(SafeTest, ExplicitConversionToArithmetic) {
  auto x = MakeSafe(3);
  float rf = x;
  uint8_t ru = x;
  EXPECT_EQ(rf, 3.f);
  EXPECT_EQ(ru, 3u);
}

TEST(SafeTest, ExplicitConversionToSafe) {
  auto x = MakeSafe(3);
  Safe<float> rf = x;
  Safe<uint8_t> ru = x;
  EXPECT_EQ(rf.get(), 3.f);
  EXPECT_EQ(ru.get(), 3u);
}

TEST(SafeTest, Not) {
  uint8_t input = 3;
  auto x = ~MakeSafe(input);
  EXPECT_VALUE_OF_TYPE(x, Safe<int>);
  EXPECT_EQ(x.get(), -4);
}

TEST(SafeTest, Neg) {
  uint32_t input = 3;
  auto x = -MakeSafe(input);
  EXPECT_VALUE_OF_TYPE(x, Safe<int64_t>);
  EXPECT_EQ(x.get(), -3);
}

TEST(SafeTest, Shift) {
  {
    Safe<uint8_t> x = 5;
    auto rv = x << 6;
    EXPECT_VALUE_OF_TYPE(rv, Safe<int>);
    EXPECT_EQ(rv, 320);
  }
}

TEST(SafeTest, BinaryArithmetic) {
  auto max_u32 = Limits<uint32_t>::Max;
  auto max_s8 = Limits<int8_t>::Max;
  auto min_s8 = Limits<int8_t>::Min;
  auto max_u8 = Limits<uint8_t>::Max;

  // signed() + signed()
  {
    auto rv = max_s8 + MakeSafe(min_s8);
    EXPECT_VALUE_OF_TYPE(rv, Safe<int>);
    EXPECT_EQ(rv.get(), -1);
  }
  {
    auto rv = MakeSafe(-5) * MakeSafe(max_s8);
    EXPECT_VALUE_OF_TYPE(rv, Safe<int>);
    EXPECT_EQ(rv.get(), -635);
  }

  // unsigned() + unsigned()
  {
    auto rv = 2u + MakeSafe(max_u8);
    EXPECT_VALUE_OF_TYPE(rv, Safe<unsigned int>);
    EXPECT_EQ(rv.get(), INT64_C(0x101));
  }
  {
    auto rv = max_u8 + MakeSafe(max_u8);
    EXPECT_VALUE_OF_TYPE(rv, Safe<int>);
    EXPECT_EQ(rv.get(), INT64_C(0x1FE));
  }

  // unsigned(32bit) + signed(<=32bit)
  {
    auto rv = max_u32 + MakeSafe(max_s8);
    EXPECT_VALUE_OF_TYPE(rv, Safe<int64_t>);
    EXPECT_EQ(rv.get(), INT64_C(0x10000007E));

    rv = max_u32 + MakeSafe(min_s8);
    EXPECT_EQ(rv.get(), INT64_C(0xFFFFFF7F));
  }

  // floating-point
  {
    auto rv = 2 - MakeSafe(0.5);
    EXPECT_VALUE_OF_TYPE(rv, Safe<double>);
    EXPECT_EQ(rv.get(), 1.5);
  }
  {
    auto rv = 1.f / MakeSafe(-2);
    EXPECT_VALUE_OF_TYPE(rv, Safe<float>);
    EXPECT_EQ(rv.get(), -0.5);
  }

  // Addition of mixed sign 64-bit integers fails to compile:
  // auto rv = MakeSafe(UINT64_C(2)) + INT64_C(2);
}

TEST(SafeTest, Compare) {
  {
    auto lhs = MakeSafe(0xFFFFFFFFu);
    auto rhs = -100;
    EXPECT_FALSE(lhs == rhs);
    EXPECT_NE(lhs, rhs);
    EXPECT_LT(rhs, lhs);
    EXPECT_LE(rhs, lhs);
    EXPECT_GE(lhs, rhs);
    EXPECT_GT(lhs, rhs);
  }
  {
    auto lhs = -MakeSafe(4.f);
    auto rhs = -4;
    EXPECT_EQ(lhs, rhs);
    EXPECT_FALSE(lhs != rhs);
    EXPECT_GE(lhs, rhs);
    EXPECT_LE(lhs, rhs);
    EXPECT_GE(rhs, lhs);
    EXPECT_LE(rhs, lhs);
    EXPECT_FALSE(rhs < lhs);
    EXPECT_FALSE(rhs > lhs);
    EXPECT_FALSE(lhs < rhs);
    EXPECT_FALSE(lhs > rhs);
  }
}

TEST(SafeTest, CompountAssignment) {
  {
    Safe<uint8_t> x = 7;
    x &= ~2;
    EXPECT_EQ(x, 5);
  }
  {
    Safe<int> x = -7;
    x *= 2;
    EXPECT_EQ(x, -14);
  }
  {
    Safe<uint32_t> x = 0x0FF00000u;
    x <<= 6;
    EXPECT_EQ(x, 0xFC000000u);
  }
  {
    Safe<uint32_t> x = 0xFF80000Fu;
    x >>= 22;
    EXPECT_EQ(x, 0x000003FE);
  }
  {
    Safe<int16_t> x = -4;
    x >>= 1;
    EXPECT_EQ(x, -2);
  }
}

TEST(SafeTest, SignConversion) {
  static_assert(TsAreSame<Safe<int8_t>, TMakeSigned<Safe<uint8_t>>>, "!");
  static_assert(TsAreSame<Safe<uint8_t>, TMakeUnsigned<Safe<int8_t>>>, "!");

  {
    auto x = ToSigned(MakeSafe(3u));
    EXPECT_VALUE_OF_TYPE(x, Safe<int>);
    EXPECT_EQ(x, 3);
  }
  {
    auto x = ToSigned(MakeSafe(-3));
    EXPECT_VALUE_OF_TYPE(x, Safe<int>);
    EXPECT_EQ(x, -3);
  }

  {
    auto x = ToUnsigned(MakeSafe(3));
    EXPECT_VALUE_OF_TYPE(x, Safe<unsigned int>);
    EXPECT_EQ(x, 3);
  }
}

TEST(SafeTest, Abs) {
  {
    auto x = Abs(MakeSafe(-3));
    EXPECT_VALUE_OF_TYPE(x, Safe<int>);
    EXPECT_EQ(x, 3);

    x = Abs(MakeSafe(3));
    EXPECT_EQ(x, 3);
  }

  {
    auto x = AbsToUnsigned(MakeSafe(-3));
    EXPECT_VALUE_OF_TYPE(x, Safe<unsigned int>);
    EXPECT_EQ(x, 3);
  }
}

} // namespace stp
