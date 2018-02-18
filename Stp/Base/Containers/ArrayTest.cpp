// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/Array.h"

#include "Base/Test/GMock.h"
#include "Base/Test/GTest.h"

namespace stp {

using testing::_;
using testing::Truly;

TEST(ArrayTest, Basic) {
  auto array = makeArray<int>(2, 3, 4);
  EXPECT_EQ(3, array.size());

  EXPECT_EQ(2, array[0]);
  EXPECT_EQ(3, array[1]);
  EXPECT_EQ(4, array[2]);

  EXPECT_EQ(2, array.getFirst());
  EXPECT_EQ(4, array.getLast());

  int tmp = 2;
  for (int x : array) {
    EXPECT_EQ(tmp, x);
    ++tmp;
  }

  // Test this[index] assignment.
  array[2] = 0;
  EXPECT_EQ(0, array[2]);
}

TEST(ArrayTest, Find) {
  auto array = makeArray(2, 2, 4, 5, 6, 7, 8);

  EXPECT_EQ(-1, array.IndexOf(0));
  EXPECT_EQ(0, array.IndexOf(2));
  EXPECT_EQ(3, array.IndexOf(5));
  EXPECT_EQ(6, array.IndexOf(8));
  EXPECT_EQ(-1, array.IndexOf(10));

  EXPECT_EQ(-1, array.LastIndexOf(0));
  EXPECT_EQ(1, array.LastIndexOf(2));
  EXPECT_EQ(3, array.LastIndexOf(5));
  EXPECT_EQ(6, array.LastIndexOf(8));
  EXPECT_EQ(-1, array.LastIndexOf(10));

  EXPECT_FALSE(array.Contains(0));
  EXPECT_TRUE(array.Contains(2));
  EXPECT_TRUE(array.Contains(5));
  EXPECT_TRUE(array.Contains(8));
  EXPECT_FALSE(array.Contains(10));
}

} // namespace stp
