// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/Sign.h"

#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {

TEST(SignTest, IsNegativeValue) {
  EXPECT_TRUE(isNegative(-1));
  EXPECT_TRUE(isNegative(Limits<int>::Min));
  EXPECT_FALSE(isNegative(Limits<unsigned>::Min));
  EXPECT_TRUE(isNegative(-Limits<double>::Max));
  EXPECT_FALSE(isNegative(0));
  EXPECT_FALSE(isNegative(1));
  EXPECT_FALSE(isNegative(0u));
  EXPECT_FALSE(isNegative(1u));
  EXPECT_FALSE(isNegative(Limits<int>::Max));
  EXPECT_FALSE(isNegative(Limits<unsigned>::Max));
  EXPECT_FALSE(isNegative(Limits<double>::Max));
}

TEST(SignTest, Signum) {
  EXPECT_EQ(0, signum(0));
  EXPECT_EQ(1, signum(1));
  EXPECT_EQ(-1, signum(-1));
  EXPECT_EQ(-1, signum(-1000));
  EXPECT_EQ(1, signum(321));

  EXPECT_EQ(0, signum(0.0f));
  EXPECT_EQ(0, signum(-0.0f));
  EXPECT_EQ(1, signum(1.0f));
  EXPECT_EQ(-1, signum(-1.0f));
  EXPECT_EQ(-1, signum(-0.5f));
  EXPECT_EQ(1, signum(0.01f));
  EXPECT_EQ(1, signum(10.0f));
  EXPECT_EQ(-1, signum(-33.0f));
  EXPECT_EQ(-1, signum(-Limits<float>::Infinity));
  EXPECT_EQ(1, signum(Limits<float>::Infinity));
}

} // namespace stp
