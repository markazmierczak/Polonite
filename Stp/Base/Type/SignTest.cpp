// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/Sign.h"

#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {

TEST(SignTest, IsNegativeValue) {
  EXPECT_TRUE(IsNegative(-1));
  EXPECT_TRUE(IsNegative(Limits<int>::Min));
  EXPECT_FALSE(IsNegative(Limits<unsigned>::Min));
  EXPECT_TRUE(IsNegative(-Limits<double>::Max));
  EXPECT_FALSE(IsNegative(0));
  EXPECT_FALSE(IsNegative(1));
  EXPECT_FALSE(IsNegative(0u));
  EXPECT_FALSE(IsNegative(1u));
  EXPECT_FALSE(IsNegative(Limits<int>::Max));
  EXPECT_FALSE(IsNegative(Limits<unsigned>::Max));
  EXPECT_FALSE(IsNegative(Limits<double>::Max));
}

TEST(SignTest, Signum) {
  EXPECT_EQ(0, Signum(0));
  EXPECT_EQ(1, Signum(1));
  EXPECT_EQ(-1, Signum(-1));
  EXPECT_EQ(-1, Signum(-1000));
  EXPECT_EQ(1, Signum(321));

  EXPECT_EQ(0, Signum(0.0f));
  EXPECT_EQ(0, Signum(-0.0f));
  EXPECT_EQ(1, Signum(1.0f));
  EXPECT_EQ(-1, Signum(-1.0f));
  EXPECT_EQ(-1, Signum(-0.5f));
  EXPECT_EQ(1, Signum(0.01f));
  EXPECT_EQ(1, Signum(10.0f));
  EXPECT_EQ(-1, Signum(-33.0f));
  EXPECT_EQ(-1, Signum(-Limits<float>::Infinity));
  EXPECT_EQ(1, Signum(Limits<float>::Infinity));
}

} // namespace stp
