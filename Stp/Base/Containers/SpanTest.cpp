// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/Span.h"

#include "Base/Test/GMock.h"
#include "Base/Test/GTest.h"

namespace stp {

using testing::_;
using testing::Truly;

TEST(Span, Basic) {
  {
    Span<int> span;
    EXPECT_EQ(nullptr, span.data());
    EXPECT_EQ(0, span.size());
    EXPECT_TRUE(span.IsEmpty());
  }

  {
    int array[] = { 2, 3, 4 };
    Span<int> span = array;
    EXPECT_EQ(array, span.data());
    EXPECT_EQ(3, span.size());
    EXPECT_FALSE(span.IsEmpty());
  }

  {
    const int array[] = { 2, 3, 4 };
    Span<int> span = array;
    EXPECT_EQ(array, span.data());
    EXPECT_EQ(3, span.size());

    EXPECT_EQ(2, span[0]);
    EXPECT_EQ(3, span[1]);
    EXPECT_EQ(4, span[2]);

    EXPECT_EQ(2, span.GetFirst());
    EXPECT_EQ(4, span.GetLast());

    int tmp = 2;
    for (int x : span) {
      EXPECT_EQ(tmp, x);
      ++tmp;
    }
  }
}

TEST(Span, Find) {
  int array[] = { 2, 2, 4, 5, 6, 7, 8 };
  Span<int> span = array;

  EXPECT_EQ(-1, span.IndexOf(0));
  EXPECT_EQ(0, span.IndexOf(2));
  EXPECT_EQ(3, span.IndexOf(5));
  EXPECT_EQ(6, span.IndexOf(8));
  EXPECT_EQ(-1, span.IndexOf(10));

  EXPECT_EQ(-1, span.LastIndexOf(0));
  EXPECT_EQ(1, span.LastIndexOf(2));
  EXPECT_EQ(3, span.LastIndexOf(5));
  EXPECT_EQ(6, span.LastIndexOf(8));
  EXPECT_EQ(-1, span.LastIndexOf(10));

  EXPECT_FALSE(span.Contains(0));
  EXPECT_TRUE(span.Contains(2));
  EXPECT_TRUE(span.Contains(5));
  EXPECT_TRUE(span.Contains(8));
  EXPECT_FALSE(span.Contains(10));
}

TEST(Span, FindIndex) {
  int array[] = { 2, 2, 4, 5, 6, 7, 8 };
  int array2[] = { 11, 2, 3 };
  Span<int> span = array;
  Span<int> span2 = array2;

  auto even = [](int x) { return (x & 1) == 0; };
  auto odd = [](int x) { return (x & 1) != 0; };
  auto gt10 = [](int x) { return x > 10; };

  EXPECT_EQ(-1, span.FindIndex(gt10));
  EXPECT_EQ(0, span.FindIndex(even));
  EXPECT_EQ(3, span.FindIndex(odd));

  EXPECT_EQ(-1, span.FindLastIndex(gt10));
  EXPECT_EQ(6, span.FindLastIndex(even));
  EXPECT_EQ(5, span.FindLastIndex(odd));
  EXPECT_EQ(0, span2.FindLastIndex(gt10));

  EXPECT_FALSE(span.Exists(gt10));
  EXPECT_TRUE(span.Exists(even));
  EXPECT_TRUE(span.Exists(odd));
}

namespace {
class ForEachReceiver {
 public:
  MOCK_METHOD1(Method, void(int));
};
} // namespace

TEST(Span, ForEach) {
  int array[] = { 1, 2, 2 };
  Span<int> span = array;

  {
    ForEachReceiver recv;
    EXPECT_CALL(recv, Method(_)).Times(3);
    span.ForEach([&recv](int x) { recv.Method(x); });
  }
}

TEST(Span, MuatbleBasic) {
  {
    MutableSpan<int> span;
    EXPECT_EQ(nullptr, span.data());
    EXPECT_EQ(0, span.size());
    EXPECT_TRUE(span.IsEmpty());
  }

  {
    int array[] = { 2, 3, 4 };
    MutableSpan<int> span(array);
    EXPECT_EQ(array, span.data());
    EXPECT_EQ(3, span.size());
    EXPECT_FALSE(span.IsEmpty());
  }

  {
    int array[] = { 2, 3, 4 };
    MutableSpan<int> span = array;
    EXPECT_EQ(array, span.data());
    EXPECT_EQ(3, span.size());

    EXPECT_EQ(2, span[0]);
    EXPECT_EQ(3, span[1]);
    EXPECT_EQ(4, span[2]);

    EXPECT_EQ(2, span.GetFirst());
    EXPECT_EQ(4, span.GetLast());

    int tmp = 2;
    for (int x : span) {
      EXPECT_EQ(tmp, x);
      ++tmp;
    }

    // Test this[index] assignment.
    span[2] = 0;
    EXPECT_EQ(0, span[2]);
  }
}

TEST(Span, MutableForEach) {
  int array[] = { 1, 2, 2 };
  MutableSpan<int> span = array;

  {
    ForEachReceiver recv;
    EXPECT_CALL(recv, Method(_)).Times(3);
    span.ForEach([&recv](int x) { recv.Method(x); });
  }

  int expected[] = { 3, 4, 4 };
  MutableSpan<int> expected_span = expected;
  span.ForEach([](int& x) { x += 2; });
  EXPECT_EQ(expected_span, span);
}

} // namespace stp