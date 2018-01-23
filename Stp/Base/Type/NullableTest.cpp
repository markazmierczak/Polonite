// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/Nullable.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(NullableTest, Basic) {
  Nullable<int> x;
  EXPECT_FALSE(x.operator bool());
  EXPECT_EQ(x, nullptr);
  EXPECT_EQ(nullptr, x);
  EXPECT_NE(x, 0);
  EXPECT_NE(0, x);
  x = 2;
  EXPECT_TRUE(x.operator bool());
  EXPECT_NE(x, nullptr);
  EXPECT_NE(nullptr, x);

  EXPECT_EQ(x, 2);
  EXPECT_EQ(2, x);

  Nullable<short> y = 2;
  EXPECT_EQ(x, y);
}

} // namespace stp
