// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/FloatToInteger.h"

#include "Base/Crypto/CryptoRandom.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(FloatToIntegerTest, FloorToInt) {
  int int_min = Limits<int>::Min;

  EXPECT_EQ(-101, FloorToInt(-100.5f));
  EXPECT_EQ(0, FloorToInt(0.f));
  EXPECT_EQ(100, FloorToInt(100.5f));

  EXPECT_EQ(int_min, FloorToInt(int_min * 1.f));
}

TEST(FloatToIntegerTest, CeilToInt) {
  int int_min = Limits<int>::Min;

  EXPECT_EQ(-100, CeilToInt(-100.5f));
  EXPECT_EQ(0, CeilToInt(0.f));
  EXPECT_EQ(101, CeilToInt(100.5f));

  EXPECT_EQ(int_min, CeilToInt(int_min * 1.f));
}

TEST(FloatToIntegerTest, RoundToInt) {
  int int_min = Limits<int>::Min;

  EXPECT_EQ(-100, RoundToInt(-100.1f));
  EXPECT_EQ(-101, RoundToInt(-100.5f));
  EXPECT_EQ(-101, RoundToInt(-100.9f));
  EXPECT_EQ(0, RoundToInt(0.f));
  EXPECT_EQ(100, RoundToInt(100.1f));
  EXPECT_EQ(101, RoundToInt(100.5f));
  EXPECT_EQ(101, RoundToInt(100.9f));

  EXPECT_EQ(int_min, RoundToInt(int_min * 1.f));
}

} // namespace stp

