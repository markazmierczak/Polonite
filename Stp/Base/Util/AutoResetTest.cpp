// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/AutoReset.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(AutoResetTest, basic) {
  int var = 3;
  {
    AutoReset<int> change(borrow(var), 5);
    EXPECT_EQ(5, var);
  }
  EXPECT_EQ(3, var);
}

TEST(AutoResetTest, persist) {
  int var = 3;
  {
    AutoReset<int> change(borrow(var), 5);
    EXPECT_EQ(5, var);
    change.persist();
  }
  EXPECT_EQ(5, var);
}

} // namespace stp
