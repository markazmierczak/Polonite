// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/SaturatedMath.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(SaturatedTest, Add_32) {
  EXPECT_EQ(0, SaturatedAdd(0, 0));
  EXPECT_EQ(0x7FFFFFFF, SaturatedAdd(0x7FFFFFFF, 0x7FFFFFFF));
  EXPECT_EQ(0x7FFFFFFF, SaturatedAdd(1, 0x7FFFFFFE));
  EXPECT_EQ(0x7FFFFFFF, SaturatedAdd(0x7FFFFFFF, 0x7FFFFFFE));
  EXPECT_EQ(INT32_MIN, SaturatedAdd(INT32_MIN, INT32_MIN));
}

TEST(SaturatedTest, Sub_32) {
  EXPECT_EQ(0, SaturatedSub(0, 0));
  EXPECT_EQ(-1, SaturatedSub(1, 2));
  EXPECT_EQ(INT32_MIN, SaturatedSub(INT32_MIN, INT32_MAX));
  EXPECT_EQ(INT32_MAX, SaturatedSub(0x1000, INT32_MIN));
}

} // namespace stp
