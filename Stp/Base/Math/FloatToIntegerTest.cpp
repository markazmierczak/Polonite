// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/FloatToInteger.h"

#include "Base/Crypto/CryptoRandom.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(FloatToIntegerTest, mathFloorToInt) {
  int int_min = Limits<int>::Min;

  EXPECT_EQ(-101, mathFloorToInt(-100.5f));
  EXPECT_EQ(0, mathFloorToInt(0.f));
  EXPECT_EQ(100, mathFloorToInt(100.5f));

  EXPECT_EQ(int_min, mathFloorToInt(int_min * 1.f));
}

TEST(FloatToIntegerTest, mathCeilToInt) {
  int int_min = Limits<int>::Min;

  EXPECT_EQ(-100, mathCeilToInt(-100.5f));
  EXPECT_EQ(0, mathCeilToInt(0.f));
  EXPECT_EQ(101, mathCeilToInt(100.5f));

  EXPECT_EQ(int_min, mathCeilToInt(int_min * 1.f));
}

TEST(FloatToIntegerTest, mathRoundToInt) {
  int int_min = Limits<int>::Min;

  EXPECT_EQ(-100, mathRoundToInt(-100.1f));
  EXPECT_EQ(-101, mathRoundToInt(-100.5f));
  EXPECT_EQ(-101, mathRoundToInt(-100.9f));
  EXPECT_EQ(0, mathRoundToInt(0.f));
  EXPECT_EQ(100, mathRoundToInt(100.1f));
  EXPECT_EQ(101, mathRoundToInt(100.5f));
  EXPECT_EQ(101, mathRoundToInt(100.9f));

  EXPECT_EQ(int_min, mathRoundToInt(int_min * 1.f));
}

} // namespace stp

