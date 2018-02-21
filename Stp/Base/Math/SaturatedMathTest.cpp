// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/SaturatedMath.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(SaturatedTest, add32) {
  EXPECT_EQ(0, saturatedAdd(0, 0));
  EXPECT_EQ(0x7FFFFFFF, saturatedAdd(0x7FFFFFFF, 0x7FFFFFFF));
  EXPECT_EQ(0x7FFFFFFF, saturatedAdd(1, 0x7FFFFFFE));
  EXPECT_EQ(0x7FFFFFFF, saturatedAdd(0x7FFFFFFF, 0x7FFFFFFE));
  EXPECT_EQ(INT32_MIN, saturatedAdd(INT32_MIN, INT32_MIN));
}

TEST(SaturatedTest, sub32) {
  EXPECT_EQ(0, saturatedSub(0, 0));
  EXPECT_EQ(-1, saturatedSub(1, 2));
  EXPECT_EQ(INT32_MIN, saturatedSub(INT32_MIN, INT32_MAX));
  EXPECT_EQ(INT32_MAX, saturatedSub(0x1000, INT32_MIN));
}

} // namespace stp
