// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/Buffer.h"

#include "Base/Test/GTest.h"
#include "Base/Type/FormattableToString.h"

namespace stp {

TEST(BufferTest, Empty) {
  Buffer empty;
  EXPECT_EQ(nullptr, empty.data());
  EXPECT_EQ(0, empty.size());
  EXPECT_TRUE(empty.isEmpty());
}

TEST(BufferTest, Format) {
  constexpr byte_t bytes[] = {0x01, 0xFF, 0x02, 0xFE, 0x03, 0x80, 0x81};
  EXPECT_EQ("01FF02FE038081", formattableToString(BufferSpan(bytes)));
}

} // namespace stp
