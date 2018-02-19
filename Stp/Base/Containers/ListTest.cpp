// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/List.h"

#include "Base/Test/GMock.h"
#include "Base/Test/GTest.h"

namespace stp {

using testing::_;
using testing::Truly;

TEST(ListTest, Empty) {
  {
    List<int> list;
    EXPECT_EQ(nullptr, list.data());
    EXPECT_EQ(0, list.size());
    EXPECT_TRUE(list.isEmpty());
  }

  {
    Span<int> empty;
    List<int> list(empty);
    EXPECT_EQ(nullptr, list.data());
    EXPECT_EQ(0, list.size());
    EXPECT_TRUE(list.isEmpty());
  }
}

TEST(ListTest, Add) {
  List<int> list;

  list.Add(2);
  list.Add(3);
  list.Add(4);

  EXPECT_FALSE(list.isEmpty());
  EXPECT_EQ(3, list.size());
  EXPECT_EQ(4, list.capacity());
  EXPECT_EQ(2, list[0]);
  EXPECT_EQ(3, list[1]);
  EXPECT_EQ(4, list[2]);
}

TEST(ListTest, SpanConversion) {
  int carray[] = { 2, 3, 4};
  Span<int> input = carray;

  List<int> list(input);
  EXPECT_EQ(3, list.size());
  EXPECT_EQ(3, list.capacity());
  EXPECT_EQ(2, list[0]);
  EXPECT_EQ(3, list[1]);
  EXPECT_EQ(4, list[2]);

  Span<int> span = list;
  EXPECT_EQ(span.data(), list.data());
  EXPECT_EQ(span.size(), list.size());

  MutableSpan<int> mutable_span = list;
  EXPECT_EQ(mutable_span.data(), list.data());
  EXPECT_EQ(mutable_span.size(), list.size());

  mutable_span[1] = 10;
  EXPECT_EQ(10, list[1]);
}

} // namespace stp
