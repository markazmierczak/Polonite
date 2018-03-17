// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/BinarySearch.h"

#include "Base/Containers/Span.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(BinarySearchTest, basic) {
  int array[] = { 2, 2, 4, 5, 5, 5, 5, 6, 7, 8 };
  Span<int> span = array;
  EXPECT_EQ(-1, binarySearchInSpan(span, 1));
  EXPECT_EQ(1, binarySearchInSpan(span, 2));
  EXPECT_EQ(-3, binarySearchInSpan(span, 3));
  EXPECT_EQ(2, binarySearchInSpan(span, 4));
  EXPECT_EQ(4, binarySearchInSpan(span, 5));
  EXPECT_EQ(7, binarySearchInSpan(span, 6));
  EXPECT_EQ(9, binarySearchInSpan(span, 8));
  EXPECT_EQ(-11, binarySearchInSpan(span, 10));
}

} // namespace stp
