// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/NBits.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(NBitsTest, saturateToUnsignedNBits) {
  EXPECT_EQ(0u, saturateToUnsignedNBits(0, 1));
  EXPECT_EQ(0u, saturateToUnsignedNBits(-10, 8));
  EXPECT_EQ(0xFFFFu, saturateToUnsignedNBits(0x7FFFFFFF, 16));
  EXPECT_EQ(0xFFu, saturateToUnsignedNBits(0x7FFFFFFF, 8));
  EXPECT_EQ(0xFFu, saturateToUnsignedNBits(0xFF, 8));
  EXPECT_EQ(0xFFu, saturateToUnsignedNBits(0x100, 8));
  EXPECT_EQ(31u, saturateToUnsignedNBits(37, 5));
  EXPECT_EQ(0x7FFFFFFFu, saturateToUnsignedNBits(0xFFFFFFFFu, 31));
  EXPECT_EQ(0xFFFFFFFFu, saturateToUnsignedNBits(0xFFFFFFFFu, 32));
}

TEST(NBitsTest, saturateToSignedNBits) {
  EXPECT_EQ(0, saturateToSignedNBits(0, 1));
  EXPECT_EQ(0x7FFF, saturateToSignedNBits(0x7FFFFFFF, 16));
  EXPECT_EQ(-0x8000, saturateToSignedNBits(INT32_MIN, 16));
  EXPECT_EQ(0x7F, saturateToSignedNBits(0x7FFFFFFF, 8));
  EXPECT_EQ(-0x80, saturateToSignedNBits(INT32_MIN, 8));
  EXPECT_EQ(0x7FFFFFFF, saturateToSignedNBits(0x7FFFFFFF, 32));
  EXPECT_EQ(15, saturateToSignedNBits(37, 5));
  EXPECT_EQ(-16, saturateToSignedNBits(-37, 5));
  EXPECT_EQ(1, saturateToSignedNBits(1, 5));
  EXPECT_EQ(-1, saturateToSignedNBits(-1, 5));
}

} // namespace stp
