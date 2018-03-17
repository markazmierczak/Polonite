// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/Array.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(ArrayTest, basic) {
  auto array = makeArray<int>(2, 3, 4);
  EXPECT_EQ(3, array.size());

  EXPECT_EQ(2, array[0]);
  EXPECT_EQ(3, array[1]);
  EXPECT_EQ(4, array[2]);

  EXPECT_EQ(2, array.first());
  EXPECT_EQ(4, array.last());

  int tmp = 2;
  for (int x : array) {
    EXPECT_EQ(tmp, x);
    ++tmp;
  }

  // Test this[index] assignment.
  array[2] = 0;
  EXPECT_EQ(0, array[2]);
}

TEST(ArrayTest, indexOf) {
  auto array = makeArray(2, 2, 4, 5, 6, 7, 8);

  EXPECT_EQ(-1, array.indexOf(0));
  EXPECT_EQ(0, array.indexOf(2));
  EXPECT_EQ(3, array.indexOf(5));
  EXPECT_EQ(6, array.indexOf(8));
  EXPECT_EQ(-1, array.indexOf(10));

  EXPECT_EQ(-1, array.lastIndexOf(0));
  EXPECT_EQ(1, array.lastIndexOf(2));
  EXPECT_EQ(3, array.lastIndexOf(5));
  EXPECT_EQ(6, array.lastIndexOf(8));
  EXPECT_EQ(-1, array.lastIndexOf(10));

  EXPECT_FALSE(array.contains(0));
  EXPECT_TRUE(array.contains(2));
  EXPECT_TRUE(array.contains(5));
  EXPECT_TRUE(array.contains(8));
  EXPECT_FALSE(array.contains(10));
}

} // namespace stp
