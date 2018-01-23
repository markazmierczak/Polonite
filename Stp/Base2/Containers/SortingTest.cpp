// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/Sorting.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(Sorting, Reverse) {
  int orig[] = { 1, 2, 3, 4, 5, 6, 6, 7 };
  int reversed[] = { 7, 6, 6, 5, 4, 3, 2, 1 };
  MutableSpan<int> porig = orig;
  MutableSpan<int> preversed = reversed;
  Reverse(porig);
  EXPECT_EQ(preversed, porig);
}

TEST(Sorting, Sort) {
  int sorted[] = { 2, 2, 4, 5, 5, 5, 5, 6, 7, 8 };
  int unsorted[] = { 5, 4, 2, 5, 5, 6, 7, 5, 2, 8 };
  MutableSpan<int> psorted = sorted;
  MutableSpan<int> punsorted = unsorted;
  Sort(punsorted);
  EXPECT_EQ(psorted, punsorted);
}

} // namespace stp
