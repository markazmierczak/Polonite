// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/BinarySearch.h"

#include "Base/Containers/Span.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(BinarySearch, BinarySearch) {
  int array[] = { 2, 2, 4, 5, 5, 5, 5, 6, 7, 8 };
  Span<int> span = array;
  EXPECT_EQ(-1, BinarySearch(span, 1));
  EXPECT_EQ(1, BinarySearch(span, 2));
  EXPECT_EQ(-3, BinarySearch(span, 3));
  EXPECT_EQ(2, BinarySearch(span, 4));
  EXPECT_EQ(4, BinarySearch(span, 5));
  EXPECT_EQ(7, BinarySearch(span, 6));
  EXPECT_EQ(9, BinarySearch(span, 8));
  EXPECT_EQ(-11, BinarySearch(span, 10));
}

} // namespace stp
