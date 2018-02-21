// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/RawFloat.h"

#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {

TEST(RawFloatTest, Values) {
  EXPECT_EQ(RawFloat(1.f).toBits(), RawFloat(1.f).toBits());
  EXPECT_EQ(RawFloat(Limits<float>::Min).toBits(), Limits<RawFloat>::Min.toBits());
  EXPECT_EQ(RawFloat(Limits<float>::Max).toBits(), Limits<RawFloat>::Max.toBits());
  EXPECT_EQ(RawFloat(Limits<float>::SmallestNormal).toBits(),
            Limits<RawFloat>::SmallestNormal.toBits());
  EXPECT_EQ(RawFloat(Limits<float>::Infinity).toBits(), Limits<RawFloat>::Infinity.toBits());
  EXPECT_EQ(RawFloat(Limits<float>::Epsilon).toBits(), Limits<RawFloat>::Epsilon.toBits());
  EXPECT_EQ(RawFloat(Limits<float>::NaN).toBits(), Limits<RawFloat>::NaN.toBits());

  EXPECT_EQ(RawDouble(1.0).toBits(), RawDouble(1.0).toBits());
  EXPECT_EQ(RawDouble(Limits<double>::Min).toBits(), Limits<RawDouble>::Min.toBits());
  EXPECT_EQ(RawDouble(Limits<double>::Max).toBits(), Limits<RawDouble>::Max.toBits());
  EXPECT_EQ(RawDouble(Limits<double>::SmallestNormal).toBits(),
            Limits<RawDouble>::SmallestNormal.toBits());
  EXPECT_EQ(RawDouble(Limits<double>::Infinity).toBits(), Limits<RawDouble>::Infinity.toBits());
  EXPECT_EQ(RawDouble(Limits<double>::Epsilon).toBits(), Limits<RawDouble>::Epsilon.toBits());
  EXPECT_EQ(RawDouble(Limits<double>::NaN).toBits(), Limits<RawDouble>::NaN.toBits());
}

} // namespace stp
