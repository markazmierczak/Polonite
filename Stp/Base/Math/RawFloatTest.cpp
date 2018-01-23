// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/RawFloat.h"

#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {

TEST(RawFloatTest, Values) {
  EXPECT_EQ(RawFloat(1.f).ToBits(), RawFloat(1.f).ToBits());
  EXPECT_EQ(RawFloat(Limits<float>::Min).ToBits(), Limits<RawFloat>::Min.ToBits());
  EXPECT_EQ(RawFloat(Limits<float>::Max).ToBits(), Limits<RawFloat>::Max.ToBits());
  EXPECT_EQ(RawFloat(Limits<float>::SmallestNormal).ToBits(),
            Limits<RawFloat>::SmallestNormal.ToBits());
  EXPECT_EQ(RawFloat(Limits<float>::Infinity).ToBits(), Limits<RawFloat>::Infinity.ToBits());
  EXPECT_EQ(RawFloat(Limits<float>::Epsilon).ToBits(), Limits<RawFloat>::Epsilon.ToBits());
  EXPECT_EQ(RawFloat(Limits<float>::NaN).ToBits(), Limits<RawFloat>::NaN.ToBits());

  EXPECT_EQ(RawDouble(1.0).ToBits(), RawDouble(1.0).ToBits());
  EXPECT_EQ(RawDouble(Limits<double>::Min).ToBits(), Limits<RawDouble>::Min.ToBits());
  EXPECT_EQ(RawDouble(Limits<double>::Max).ToBits(), Limits<RawDouble>::Max.ToBits());
  EXPECT_EQ(RawDouble(Limits<double>::SmallestNormal).ToBits(),
            Limits<RawDouble>::SmallestNormal.ToBits());
  EXPECT_EQ(RawDouble(Limits<double>::Infinity).ToBits(), Limits<RawDouble>::Infinity.ToBits());
  EXPECT_EQ(RawDouble(Limits<double>::Epsilon).ToBits(), Limits<RawDouble>::Epsilon.ToBits());
  EXPECT_EQ(RawDouble(Limits<double>::NaN).ToBits(), Limits<RawDouble>::NaN.ToBits());
}

} // namespace stp
