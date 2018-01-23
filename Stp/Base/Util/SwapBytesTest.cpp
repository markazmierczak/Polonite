// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/SwapBytes.h"

#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {

TEST(SwapBytesTest, Unsigned) {
  unsigned char x = 0xAC;
  EXPECT_EQ(x, SwapBytes(x));

  {
    uint16_t x = 0x1234;
    uint16_t r = 0x3412;
    EXPECT_EQ(r, SwapBytes(x));
  }
  {
    uint32_t x = 0x1234ABCD;
    uint32_t r = 0xCDAB3412;
    EXPECT_EQ(r, SwapBytes(x));
  }
  {
    uint64_t x = UINT64_C(0x1234ABCD567890EF);
    uint64_t r = UINT64_C(0xEF907856CDAB3412);
    EXPECT_EQ(r, SwapBytes(x));
  }
}

TEST(SwapBytesTest, Signed) {
  signed char x = -70;
  EXPECT_EQ(x, SwapBytes(x));

  {
    int16_t x = 0x1234;
    int16_t r = 0x3412;
    EXPECT_EQ(r, SwapBytes(x));
  }
  {
    int32_t x = 0x80;
    int32_t r = Limits<int32_t>::Min;
    EXPECT_EQ(r, SwapBytes(x));
  }
}

} // namespace stp
