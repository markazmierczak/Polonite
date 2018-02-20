// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/Fixed.h"

#include "Base/Math/Math.h"
#include "Base/Math/MathConstants.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(FixedTest, FromInt) {
  EXPECT_EQ(2 << 6, Fixed26_6(2).ToBits());
  EXPECT_EQ(2 << 16, Fixed16(2).ToBits());
  EXPECT_EQ(0xFFFF0600u, static_cast<uint32_t>(Fixed26_6(-1000).ToBits()));
  EXPECT_EQ(0xFC180000u, static_cast<uint32_t>(Fixed16(-1000).ToBits()));
}

TEST(FixedTest, FromFloat) {
  EXPECT_EQ(3 << 5, Fixed26_6(1.5f).ToBits());
  EXPECT_EQ(3 << 15, Fixed16(1.5f).ToBits());
  EXPECT_EQ(0xFFFF0630u, static_cast<uint32_t>(Fixed26_6(-999.25).ToBits()));
  EXPECT_EQ(0xFC184000u, static_cast<uint32_t>(Fixed16(-999.75f).ToBits()));
}

TEST(FixedTest, Floor) {
  constexpr Fixed16 Epsilon = Limits<Fixed16>::Epsilon;
  EXPECT_EQ(Fixed16(1), Floor(Fixed16(1.5f)));
  EXPECT_EQ(1, FloorToInt(Fixed16(1.5f)));
  EXPECT_EQ(1, FloorToInt(Fixed16(2) - Epsilon));
  EXPECT_EQ(1, FloorToInt(Fixed16(1)));
  EXPECT_EQ(1, FloorToInt(Fixed16(1) + Epsilon));
  EXPECT_EQ(-1, FloorToInt(Fixed16(-1)));
  EXPECT_EQ(-2, FloorToInt(-Fixed16(1) - Epsilon));
  EXPECT_EQ(-2, FloorToInt(Fixed16(-1.5f)));
  EXPECT_EQ(-2, FloorToInt(-Fixed16(1) * 2 + Epsilon));
}

TEST(FixedTest, Ceil) {
  const Fixed16 Epsilon = Limits<Fixed16>::Epsilon;
  EXPECT_EQ(Fixed16(1) * 2, Ceil(Fixed16(1.5f)));
  EXPECT_EQ(2, CeilToInt(Fixed16(1.5f)));
  EXPECT_EQ(2, CeilToInt(Fixed16(1) * 2 - Epsilon));
  EXPECT_EQ(1, CeilToInt(Fixed16(1)));
  EXPECT_EQ(2, CeilToInt(Fixed16(1) + Epsilon));
  EXPECT_EQ(-1, CeilToInt(Fixed16(-1)));
  EXPECT_EQ(-1, CeilToInt(-Fixed16(1) - Epsilon));
  EXPECT_EQ(-1, CeilToInt(Fixed16(-1.5f)));
  EXPECT_EQ(-1, CeilToInt(-Fixed16(1) * 2 + Epsilon));
}

TEST(FixedTest, Round) {
  const Fixed16 Epsilon = Limits<Fixed16>::Epsilon;
  EXPECT_EQ(Fixed16(2), Round(Fixed16(1.5f)));
  EXPECT_EQ(2, RoundToInt(Fixed16(1.5f)));
  EXPECT_EQ(2, RoundToInt(Fixed16(1) * 2 - Epsilon));
  EXPECT_EQ(1, RoundToInt(Fixed16(1.5f) - Epsilon));
  EXPECT_EQ(1, RoundToInt(Fixed16(1)));
  EXPECT_EQ(1, RoundToInt(Fixed16(1) + Epsilon));
  EXPECT_EQ(-1, RoundToInt(Fixed16(-1)));
  EXPECT_EQ(-1, RoundToInt(Fixed16(-1.5f) + Epsilon));
  EXPECT_EQ(-1, RoundToInt(-Fixed16(1) - Epsilon));
  EXPECT_EQ(-2, RoundToInt(Fixed16(-1.5f)));
  EXPECT_EQ(-2, RoundToInt(-Fixed16(1) * 2 + Epsilon));
}

TEST(FixedTest, Split) {
  Fixed26_6 x, integral, fractional;

  x = Fixed26_6(-123.25f);
  auto result = Decompose(x);
  EXPECT_EQ(Fixed26_6(-123.0f), result.integral);
  EXPECT_EQ(Fixed26_6(-0.25f), result.fractional);

  x = Fixed26_6(-2.0f);
  result = Decompose(x);
  EXPECT_EQ(Fixed26_6(-2.0f), result.integral);
  EXPECT_EQ(Fixed26_6(-0.0f), result.fractional);

  x = Fixed26_6(123.25f);
  result = Decompose(x);
  EXPECT_EQ(Fixed26_6(123.0f), result.integral);
  EXPECT_EQ(Fixed26_6(0.25f), result.fractional);
}

TEST(FixedTest, DivideFixed) {
  EXPECT_EQ(Fixed16(1.5f), Fixed16(15.f) / Fixed16(10.f));
  EXPECT_EQ(Fixed16(-1.5f), Fixed16(15.f) / Fixed16(-10.f));
}

TEST(FixedTest, Sqrt) {
  #if 0
  // Following code tests all possible values.
  constexpr int Point = 6;
  auto reference = [](double a) {
    double arg = a / (1 << Point);
    double rsq = Sqrt(arg);
    return static_cast<uint32_t>(rsq * (1 << Point) + 0.5);
  };

  uint32_t Max = Limits<int32_t>::Max;
  for (uint32_t i = 1; i <= Max; ++i) {
    double result = Sqrt(Fixed<Point>::FromBits(i)).ToBits();
    double expected = reference(i);
    EXPECT_NEAR(expected, result, 1.0);
  }
  #endif

  constexpr double Sqrt2Double = MathConstants<double>::Sqrt2;
  const auto Epsilon16 = Limits<Fixed16>::Epsilon;
  EXPECT_TRUE(isNear(Fixed16(Sqrt2Double), Sqrt(Fixed16(2)), Epsilon16));
  EXPECT_TRUE(isNear(Fixed16(8), Sqrt(Fixed16(64)), Epsilon16));
  EXPECT_TRUE(isNear(Fixed16(0.25f), Sqrt(Fixed16(0.0625)), Epsilon16));

  const auto Epsilon6 = Limits<Fixed26_6>::Epsilon;
  EXPECT_NEAR(Fixed26_6(Sqrt2Double).ToBits(), Sqrt(Fixed26_6(2)).ToBits(), Epsilon6.ToBits());
  EXPECT_TRUE(isNear(Fixed26_6(8), Sqrt(Fixed26_6(64)), Epsilon6));
  EXPECT_TRUE(isNear(Fixed26_6(0.25f), Sqrt(Fixed26_6(0.0625f)), Epsilon6));
}

TEST(FixedTest, Lerp) {
  EXPECT_EQ(Fixed16(0.5), lerp(Fixed16(0), Fixed16(1), 0.5));
  EXPECT_EQ(Fixed16(2), lerp(Fixed16(0), Fixed16(2), 1));
  EXPECT_EQ(Fixed16(-1), lerp(Fixed16(-1), Fixed16(2), 0));
}

TEST(FixedTest, RSqrt) {
  EXPECT_EQ(Fixed16(1), RSqrt(Fixed16(1)));
  EXPECT_EQ(Fixed16(0.5f), RSqrt(Fixed16(4)));

  #if 0
  // Following code tests all possible values.
  auto reference = [](uint32_t a) {
    double arg = a / 65536.0;
    double rsq = Sqrt(1.0 / arg);
    return static_cast<uint32_t>(rsq * 65536.0 + 0.5);
  };

  uint32_t Max = Limits<int32_t>::Max;
  for (uint32_t i = 1; i <= Max; ++i) {
    double result = RSqrt(Fixed16::FromBits(i)).ToBits();
    double expected = reference(i);
    EXPECT_NEAR(result, expected, 1.0);
  }
  #endif
}

TEST(FixedTest, Conversion) {
  auto f8 = Fixed24_8(5.25f);
  auto f16 = static_cast<Fixed16_16>(f8);
  auto f6 = static_cast<Fixed26_6>(f8);
  EXPECT_EQ(Fixed16_16(5.25f), f16);
  EXPECT_EQ(Fixed26_6(5.25f), f6);
}

} // namespace stp
