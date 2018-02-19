// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/Tuple.h"

#include "Base/Test/GTest.h"

namespace stp {
namespace {

TEST(TupleTest, Basic) {
  Tuple<int, int> x;
  EXPECT_EQ(2, x.size());

  x.template get<0>() = 0;
  x.template get<1>() = 3;
  EXPECT_EQ(0, x.template get<0>());
  EXPECT_EQ(3, x.template get<1>());

  x = makeTuple(4, 5);
  EXPECT_EQ(4, x.template get<0>());
  EXPECT_EQ(5, x.template get<1>());

  auto y = x;
  EXPECT_EQ(y, x);
  EXPECT_FALSE(y < x);
  EXPECT_FALSE(x < y);

  y = makeTuple(4, 6);
  EXPECT_NE(y, x);
  EXPECT_GT(y, x);
  EXPECT_TRUE(y > x);
  EXPECT_FALSE(x > y);
  EXPECT_FALSE(x >= y);
}

TEST(TupleTest, apply) {
  auto t = makeTuple(1, 3, 7);
  auto u = t.apply([](int A, int B, int C) { return makeTuple(A - B, B - C, C - A); });

  EXPECT_EQ(-2, u.template get<0>());
  EXPECT_EQ(-4, u.template get<1>());
  EXPECT_EQ(6, u.template get<2>());

  auto v = t.apply(
      [](int A, int B, int C) {
        return makeTuple(makeTuple(A, char('A' + A)),
                         makeTuple(B, char('A' + B)),
                         makeTuple(C, char('A' + C)));
      });

  EXPECT_EQ(makeTuple(1, 'B'), v.template get<0>());
  EXPECT_EQ(makeTuple(3, 'D'), v.template get<1>());
  EXPECT_EQ(makeTuple(7, 'H'), v.template get<2>());
}

struct SqueezedStruct {};

TEST(TupleTest, Squeezed) {
  auto t = makeTuple(1, SqueezedStruct(), 7);
  EXPECT_EQ(8u, sizeof(t));

  auto t2 = makeTuple(SqueezedStruct(), 7);
  EXPECT_EQ(4u, sizeof(t2));

  auto t3 = makeTuple(7, SqueezedStruct());
  EXPECT_EQ(4u, sizeof(t3));
}

} // namespace
} // namespace stp
