// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/NBits.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(NBitsTest, SaturateToUnsignedNBits) {
  EXPECT_EQ(0u, SaturateToUnsignedNBits(0, 1));
  EXPECT_EQ(0u, SaturateToUnsignedNBits(-10, 8));
  EXPECT_EQ(0xFFFFu, SaturateToUnsignedNBits(0x7FFFFFFF, 16));
  EXPECT_EQ(0xFFu, SaturateToUnsignedNBits(0x7FFFFFFF, 8));
  EXPECT_EQ(0xFFu, SaturateToUnsignedNBits(0xFF, 8));
  EXPECT_EQ(0xFFu, SaturateToUnsignedNBits(0x100, 8));
  EXPECT_EQ(31u, SaturateToUnsignedNBits(37, 5));
  EXPECT_EQ(0x7FFFFFFFu, SaturateToUnsignedNBits(0xFFFFFFFFu, 31));
  EXPECT_EQ(0xFFFFFFFFu, SaturateToUnsignedNBits(0xFFFFFFFFu, 32));
}

TEST(NBitsTest, SaturateToSignedNBits) {
  EXPECT_EQ(0, SaturateToSignedNBits(0, 1));
  EXPECT_EQ(0x7FFF, SaturateToSignedNBits(0x7FFFFFFF, 16));
  EXPECT_EQ(-0x8000, SaturateToSignedNBits(INT32_MIN, 16));
  EXPECT_EQ(0x7F, SaturateToSignedNBits(0x7FFFFFFF, 8));
  EXPECT_EQ(-0x80, SaturateToSignedNBits(INT32_MIN, 8));
  EXPECT_EQ(0x7FFFFFFF, SaturateToSignedNBits(0x7FFFFFFF, 32));
  EXPECT_EQ(15, SaturateToSignedNBits(37, 5));
  EXPECT_EQ(-16, SaturateToSignedNBits(-37, 5));
  EXPECT_EQ(1, SaturateToSignedNBits(1, 5));
  EXPECT_EQ(-1, SaturateToSignedNBits(-1, 5));
}

} // namespace stp
