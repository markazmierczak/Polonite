// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/Basic.h"

#include "Base/Test/GTest.h"

namespace stp {

static_assert(sizeof(wchar_t) == SIZEOF_WCHAR_T, "!");

TEST(BasicTest, AlignedByteArray) {
  EXPECT_EQ(2u, sizeof(AlignedByteArray<2, 1>));
  EXPECT_EQ(1u, alignof(AlignedByteArray<2, 1>));

  EXPECT_EQ(2u, sizeof(AlignedByteArray<2, 2>));
  EXPECT_EQ(2u, alignof(AlignedByteArray<2, 2>));

  EXPECT_EQ(64u, sizeof(AlignedByteArray<64, 32>));
  EXPECT_EQ(32u, alignof(AlignedByteArray<64, 32>));
}

} // namespace stp
