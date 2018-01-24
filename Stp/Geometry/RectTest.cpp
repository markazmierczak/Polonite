// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Geometry/Rect.h"

#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {

TEST(RectTest, Contains) {
  static const struct ContainsCase {
    int rect_x;
    int rect_y;
    int rect_width;
    int rect_height;
    int point_x;
    int point_y;
    bool contained;
  } tests[] = {
    {0, 0, 10, 10, 0, 0, true},
    {0, 0, 10, 10, 5, 5, true},
    {0, 0, 10, 10, 9, 9, true},
    {0, 0, 10, 10, 5, 10, false},
    {0, 0, 10, 10, 10, 5, false},
    {0, 0, 10, 10, -1, -1, false},
    {0, 0, 10, 10, 50, 50, false},
    {0, 0, -10, -10, 0, 0, false},
  };
  for (const auto& test : tests) {
    const ContainsCase& value = test;
    IntRect rect(value.rect_x, value.rect_y, value.rect_width, value.rect_height);
    EXPECT_EQ(value.contained, rect.Contains(value.point_x, value.point_y));
  }
}

TEST(RectTest, Intersects) {
  static const struct {
    int x1;  // rect 1
    int y1;
    int w1;
    int h1;
    int x2;  // rect 2
    int y2;
    int w2;
    int h2;
    bool intersects;
  } tests[] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, false },
    { 0, 0, 0, 0, -10, -10, 20, 20, false },
    { -10, 0, 0, 20, 0, -10, 20, 0, false },
    { 0, 0, 10, 10, 0, 0, 10, 10, true },
    { 0, 0, 10, 10, 10, 10, 10, 10, false },
    { 10, 10, 10, 10, 0, 0, 10, 10, false },
    { 10, 10, 10, 10, 5, 5, 10, 10, true },
    { 10, 10, 10, 10, 15, 15, 10, 10, true },
    { 10, 10, 10, 10, 20, 15, 10, 10, false },
    { 10, 10, 10, 10, 21, 15, 10, 10, false }
  };
  for (const auto& test : tests) {
    IntRect r1(test.x1, test.y1, test.w1, test.h1);
    IntRect r2(test.x2, test.y2, test.w2, test.h2);
    EXPECT_EQ(test.intersects, IntRect::Intersects(r1, r2));
    EXPECT_EQ(test.intersects, IntRect::Intersects(r2, r1));
  }
}

TEST(RectTest, Intersect) {
  static const struct {
    int x1;  // rect 1
    int y1;
    int w1;
    int h1;
    int x2;  // rect 2
    int y2;
    int w2;
    int h2;
    int x3;  // rect 3: the union of rects 1 and 2
    int y3;
    int w3;
    int h3;
  } tests[] = {
    { 0, 0, 0, 0,   // zeros
      0, 0, 0, 0,
      0, 0, 0, 0 },
    { 0, 0, 4, 4,   // equal
      0, 0, 4, 4,
      0, 0, 4, 4 },
    { 0, 0, 4, 4,   // neighboring
      4, 4, 4, 4,
      0, 0, 0, 0 },
    { 0, 0, 4, 4,   // overlapping corners
      2, 2, 4, 4,
      2, 2, 2, 2 },
    { 0, 0, 4, 4,   // T junction
      3, 1, 4, 2,
      3, 1, 1, 2 },
    { 3, 0, 2, 2,   // gap
      0, 0, 2, 2,
      0, 0, 0, 0 }
  };
  for (const auto& test : tests) {
    IntRect r1(test.x1, test.y1, test.w1, test.h1);
    IntRect r2(test.x2, test.y2, test.w2, test.h2);
    IntRect r3(test.x3, test.y3, test.w3, test.h3);
    IntRect ir = IntRect::Intersection(r1, r2);
    EXPECT_EQ(r3.left(), ir.left());
    EXPECT_EQ(r3.top(), ir.top());
    EXPECT_EQ(r3.width(), ir.width());
    EXPECT_EQ(r3.height(), ir.height());
  }
}

TEST(RectTest, Union) {
  static const struct Test {
    int x1;  // rect 1
    int y1;
    int w1;
    int h1;
    int x2;  // rect 2
    int y2;
    int w2;
    int h2;
    int x3;  // rect 3: the union of rects 1 and 2
    int y3;
    int w3;
    int h3;
  } tests[] = {
    { 0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0 },
    { 0, 0, 4, 4,
      0, 0, 4, 4,
      0, 0, 4, 4 },
    { 0, 0, 4, 4,
      4, 4, 4, 4,
      0, 0, 8, 8 },
    { 0, 0, 4, 4,
      0, 5, 4, 4,
      0, 0, 4, 9 },
    { 0, 0, 2, 2,
      3, 3, 2, 2,
      0, 0, 5, 5 },
    { 3, 3, 2, 2,   // reverse r1 and r2 from previous test
      0, 0, 2, 2,
      0, 0, 5, 5 },
    { 0, 0, 0, 0,   // union with empty rect
      2, 2, 2, 2,
      2, 2, 2, 2 }
  };
  for (const auto& test : tests) {
    IntRect r1(test.x1, test.y1, test.w1, test.h1);
    IntRect r2(test.x2, test.y2, test.w2, test.h2);
    IntRect r3(test.x3, test.y3, test.w3, test.h3);
    IntRect u = IntRect::Union(r1, r2);
    EXPECT_EQ(r3.left(), u.left());
    EXPECT_EQ(r3.top(), u.top());
    EXPECT_EQ(r3.width(), u.width());
    EXPECT_EQ(r3.height(), u.height());
  }
}

TEST(RectTest, Equals) {
  ASSERT_TRUE(IntRect(0, 0, 0, 0) == IntRect(0, 0, 0, 0));
  ASSERT_TRUE(IntRect(1, 2, 3, 4) == IntRect(1, 2, 3, 4));
  ASSERT_FALSE(IntRect(0, 0, 0, 0) == IntRect(0, 0, 0, 1));
  ASSERT_FALSE(IntRect(0, 0, 0, 0) == IntRect(0, 0, 1, 0));
  ASSERT_FALSE(IntRect(0, 0, 0, 0) == IntRect(0, 1, 0, 0));
  ASSERT_FALSE(IntRect(0, 0, 0, 0) == IntRect(1, 0, 0, 0));
}

TEST(RectTest, IsEmpty) {
  EXPECT_TRUE(IntRect(0, 0, 0, 0).IsEmpty());
  EXPECT_TRUE(IntRect(0, 0, 10, 0).IsEmpty());
  EXPECT_TRUE(IntRect(0, 0, 0, 10).IsEmpty());
  EXPECT_FALSE(IntRect(0, 0, 10, 10).IsEmpty());
}

TEST(RectTest, GetCenterPoint) {
  IntPoint2 center;

  // When origin is (0, 0).
  center = IntRect(0, 0, 20, 20).GetCenterPoint();
  EXPECT_TRUE(center == IntPoint2(10, 10));

  // When origin is even.
  center = IntRect(10, 10, 20, 20).GetCenterPoint();
  EXPECT_TRUE(center == IntPoint2(20, 20));

  // When origin is odd.
  center = IntRect(11, 11, 20, 20).GetCenterPoint();
  EXPECT_TRUE(center == IntPoint2(21, 21));

  // When 0 width or height.
  center = IntRect(10, 10, 0, 20).GetCenterPoint();
  EXPECT_TRUE(center == IntPoint2(10, 20));
  center = IntRect(10, 10, 20, 0).GetCenterPoint();
  EXPECT_TRUE(center == IntPoint2(20, 10));

  // When an odd size.
  center = IntRect(10, 10, 21, 21).GetCenterPoint();
  EXPECT_TRUE(center == IntPoint2(20, 20));

  // When an odd size and position.
  center = IntRect(11, 11, 21, 21).GetCenterPoint();
  EXPECT_TRUE(center == IntPoint2(21, 21));
}

TEST(RectTest, BoundingRect) {
  struct {
    IntPoint2 a;
    IntPoint2 b;
    IntRect expected;
  } tests[] = {
    // If point B dominates A, then A should be the origin.
    { IntPoint2(4, 6), IntPoint2(4, 6), IntRect(4, 6, 0, 0) },
    { IntPoint2(4, 6), IntPoint2(8, 6), IntRect(4, 6, 4, 0) },
    { IntPoint2(4, 6), IntPoint2(4, 9), IntRect(4, 6, 0, 3) },
    { IntPoint2(4, 6), IntPoint2(8, 9), IntRect(4, 6, 4, 3) },
    // If point A dominates B, then B should be the origin.
    { IntPoint2(4, 6), IntPoint2(4, 6), IntRect(4, 6, 0, 0) },
    { IntPoint2(8, 6), IntPoint2(4, 6), IntRect(4, 6, 4, 0) },
    { IntPoint2(4, 9), IntPoint2(4, 6), IntRect(4, 6, 0, 3) },
    { IntPoint2(8, 9), IntPoint2(4, 6), IntRect(4, 6, 4, 3) },
    // If neither point dominates, then the origin is a combination of the two.
    { IntPoint2(4, 6), IntPoint2(6, 4), IntRect(4, 4, 2, 2) },
    { IntPoint2(-4, -6), IntPoint2(-6, -4), IntRect(-6, -6, 2, 2) },
    { IntPoint2(-4, 6), IntPoint2(6, -4), IntRect(-4, -4, 10, 10) },
  };

  for (const auto& test : tests) {
    IntRect actual = IntRect::Enclose(test.a, test.b);
    EXPECT_EQ(test.expected, actual);
  }
}

TEST(RectTest, Offset) {
  IntRect i(1, 2, 3, 4);

  EXPECT_EQ(IntRect(2, 1, 3, 4), (i + IntVector2(1, -1)));
  i += IntVector2(1, -1);
  EXPECT_EQ(IntRect(2, 1, 3, 4), i);
  EXPECT_EQ(IntRect(1, 2, 3, 4), (i - IntVector2(1, -1)));
  i -= IntVector2(1, -1);
  EXPECT_EQ(IntRect(1, 2, 3, 4), i);
}

TEST(RectTest, Corners) {
  IntRect i(1, 2, 3, 4);

  EXPECT_EQ(IntPoint2(1, 2), i.GetTopLeft());
  EXPECT_EQ(IntPoint2(4, 2), i.GetTopRight());
  EXPECT_EQ(IntPoint2(1, 6), i.GetBottomLeft());
  EXPECT_EQ(IntPoint2(4, 6), i.GetBottomRight());
}

} // namespace stp
