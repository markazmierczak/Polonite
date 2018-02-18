// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/Half.h"

#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {

TEST(HalfTest, Values) {
  EXPECT_EQ(0x0400u, Limits<Half>::SmallestNormal.ToBits());
  EXPECT_EQ(0x0001u, Limits<Half>::SmallestSubnormal.ToBits());
  EXPECT_EQ(0x7BFFu, Limits<Half>::Max.ToBits());
  EXPECT_EQ(0xFBFFu, Limits<Half>::Min.ToBits());
  EXPECT_EQ(0x7C00u, Limits<Half>::Infinity.ToBits());
  EXPECT_EQ(0xFC00u, (-Limits<Half>::Infinity).ToBits());
  EXPECT_EQ(0x1400u, Limits<Half>::Epsilon.ToBits());
  EXPECT_EQ(0x7FFFu, Limits<Half>::NaN.ToBits());
}

TEST(HalfTest, Comparison) {
  EXPECT_EQ(static_cast<Half>(0.f), static_cast<Half>(-0.f));
  EXPECT_NE(static_cast<Half>(1.f), static_cast<Half>(2.f));
  EXPECT_EQ(static_cast<Half>(2.f), static_cast<Half>(2.f));

  EXPECT_EQ(Limits<Half>::Infinity, Limits<Half>::Infinity);
  EXPECT_FALSE(Limits<Half>::Infinity == Limits<Half>::NaN);
  EXPECT_FALSE(Limits<Half>::Infinity != Limits<Half>::NaN);
  EXPECT_FALSE(Limits<Half>::NaN == Limits<Half>::NaN);
  EXPECT_FALSE(Limits<Half>::NaN != Limits<Half>::NaN);
}

TEST(HalfTest, Conversion) {
  EXPECT_EQ(Limits<Half>::Infinity, static_cast<Half>(Limits<float>::Infinity));
  EXPECT_EQ(-Limits<Half>::Infinity, static_cast<Half>(-Limits<float>::Infinity));
  EXPECT_NE(static_cast<Half>(0.f).ToBits(), static_cast<Half>(-0.f).ToBits());
}

TEST(HalfTest, Finite) {
  Half inf = Limits<Half>::Infinity;
  EXPECT_TRUE(isInfinity(inf));
  EXPECT_FALSE(isFinite(inf));
  EXPECT_FALSE(isNaN(inf));
  EXPECT_FALSE(IsNormal(inf));

  Half nan = Limits<Half>::NaN;
  EXPECT_FALSE(isInfinity(nan));
  EXPECT_FALSE(isFinite(nan));
  EXPECT_TRUE(isNaN(nan));
  EXPECT_FALSE(IsNormal(nan));

  Half one = static_cast<Half>(1.f);
  EXPECT_FALSE(isInfinity(one));
  EXPECT_TRUE(isFinite(one));
  EXPECT_FALSE(isNaN(one));
  EXPECT_TRUE(IsNormal(one));

  Half eps = Limits<Half>::Epsilon;
  EXPECT_FALSE(isInfinity(eps));
  EXPECT_TRUE(isFinite(eps));
  EXPECT_FALSE(isNaN(eps));
  EXPECT_TRUE(IsNormal(eps));
}

} // namespace stp
