// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Geometry/Quad2.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(QuadTest, Construction) {
  // Verify constructors.
  Point2 a(1, 1);
  Point2 b(2, 1);
  Point2 c(2, 2);
  Point2 d(1, 2);
  Quad2 q3(a, b, c, d);

  EXPECT_EQ(q3.p[0], a);
  EXPECT_EQ(q3.p[1], b);
  EXPECT_EQ(q3.p[2], c);
  EXPECT_EQ(q3.p[3], d);
}

TEST(QuadTest, AddingVectors) {
  Point2 a(1, 1);
  Point2 b(2, 1);
  Point2 c(2, 2);
  Point2 d(1, 2);
  Vector2 v(3.5f, -2.5f);

  Quad2 q1(a, b, c, d);
  Quad2 added = q1 + v;
  q1 += v;
  Quad2 expected1(
      Point2(4.5f, -1.5f),
      Point2(5.5f, -1.5f),
      Point2(5.5f, -0.5f),
      Point2(4.5f, -0.5f));

  EXPECT_EQ(expected1, added);
  EXPECT_EQ(expected1, q1);

  Quad2 q2(a, b, c, d);
  Quad2 subtracted = q2 - v;
  q2 -= v;
  Quad2 expected2(
      Point2(-2.5f, 3.5f),
      Point2(-1.5f, 3.5f),
      Point2(-1.5f, 4.5f),
      Point2(-2.5f, 4.5f));

  EXPECT_EQ(expected2, subtracted);
  EXPECT_EQ(expected2, q2);

  Quad2 q3(a, b, c, d);
  q3 += v;
  q3 -= v;
  EXPECT_EQ(Quad2(a, b, c, d), q3);
  EXPECT_EQ(q3, (q3 + v - v));
}

TEST(QuadTest, IsRectilinear) {
  Point2 a(1, 1);
  Point2 b(2, 1);
  Point2 c(2, 2);
  Point2 d(1, 2);
  Vector2 v(3.5f, -2.5f);

  EXPECT_TRUE(Quad2().IsRectilinear());
  EXPECT_TRUE(Quad2(a, b, c, d).IsRectilinear());
  EXPECT_TRUE((Quad2(a, b, c, d) + v).IsRectilinear());

  float epsilon = Limits<float>::Epsilon;
  Point2 a2(1 + epsilon / 2, 1 + epsilon / 2);
  Point2 b2(2 + epsilon / 2, 1 + epsilon / 2);
  Point2 c2(2 + epsilon / 2, 2 + epsilon / 2);
  Point2 d2(1 + epsilon / 2, 2 + epsilon / 2);
  EXPECT_TRUE(Quad2(a2, b, c, d).IsRectilinear());
  EXPECT_TRUE((Quad2(a2, b, c, d) + v).IsRectilinear());
  EXPECT_TRUE(Quad2(a, b2, c, d).IsRectilinear());
  EXPECT_TRUE((Quad2(a, b2, c, d) + v).IsRectilinear());
  EXPECT_TRUE(Quad2(a, b, c2, d).IsRectilinear());
  EXPECT_TRUE((Quad2(a, b, c2, d) + v).IsRectilinear());
  EXPECT_TRUE(Quad2(a, b, c, d2).IsRectilinear());
  EXPECT_TRUE((Quad2(a, b, c, d2) + v).IsRectilinear());

  struct {
    Point2 a_off, b_off, c_off, d_off;
  } tests[] = {
    {
      Point2(1, 1.00001f),
      Point2(2, 1.00001f),
      Point2(2, 2.00001f),
      Point2(1, 2.00001f)
    },
    {
      Point2(1.00001f, 1),
      Point2(2.00001f, 1),
      Point2(2.00001f, 2),
      Point2(1.00001f, 2)
    },
    {
      Point2(1.00001f, 1.00001f),
      Point2(2.00001f, 1.00001f),
      Point2(2.00001f, 2.00001f),
      Point2(1.00001f, 2.00001f)
    },
    {
      Point2(1, 0.99999f),
      Point2(2, 0.99999f),
      Point2(2, 1.99999f),
      Point2(1, 1.99999f)
    },
    {
      Point2(0.99999f, 1),
      Point2(1.99999f, 1),
      Point2(1.99999f, 2),
      Point2(0.99999f, 2)
    },
    {
      Point2(0.99999f, 0.99999f),
      Point2(1.99999f, 0.99999f),
      Point2(1.99999f, 1.99999f),
      Point2(0.99999f, 1.99999f)
    }
  };

  for (const auto& test : tests) {
    Point2 a_off = test.a_off;
    Point2 b_off = test.b_off;
    Point2 c_off = test.c_off;
    Point2 d_off = test.d_off;

    EXPECT_FALSE(Quad2(a_off, b, c, d).IsRectilinear());
    EXPECT_FALSE((Quad2(a_off, b, c, d) + v).IsRectilinear());
    EXPECT_FALSE(Quad2(a, b_off, c, d).IsRectilinear());
    EXPECT_FALSE((Quad2(a, b_off, c, d) + v).IsRectilinear());
    EXPECT_FALSE(Quad2(a, b, c_off, d).IsRectilinear());
    EXPECT_FALSE((Quad2(a, b, c_off, d) + v).IsRectilinear());
    EXPECT_FALSE(Quad2(a, b, c, d_off).IsRectilinear());
    EXPECT_FALSE((Quad2(a, b, c, d_off) + v).IsRectilinear());
    EXPECT_FALSE(Quad2(a_off, b, c_off, d).IsRectilinear());
    EXPECT_FALSE((Quad2(a_off, b, c_off, d) + v).IsRectilinear());
    EXPECT_FALSE(Quad2(a, b_off, c, d_off).IsRectilinear());
    EXPECT_FALSE((Quad2(a, b_off, c, d_off) + v).IsRectilinear());
    EXPECT_FALSE(Quad2(a, b_off, c_off, d_off).IsRectilinear());
    EXPECT_FALSE((Quad2(a, b_off, c_off, d_off) + v).IsRectilinear());
    EXPECT_FALSE(Quad2(a_off, b, c_off, d_off).IsRectilinear());
    EXPECT_FALSE((Quad2(a_off, b, c_off, d_off) + v).IsRectilinear());
    EXPECT_FALSE(Quad2(a_off, b_off, c, d_off).IsRectilinear());
    EXPECT_FALSE((Quad2(a_off, b_off, c, d_off) + v).IsRectilinear());
    EXPECT_FALSE(Quad2(a_off, b_off, c_off, d).IsRectilinear());
    EXPECT_FALSE((Quad2(a_off, b_off, c_off, d) + v).IsRectilinear());
    EXPECT_TRUE(Quad2(a_off, b_off, c_off, d_off).IsRectilinear());
    EXPECT_TRUE((Quad2(a_off, b_off, c_off, d_off) + v).IsRectilinear());
  }
}

TEST(QuadTest, IsCounterClockwise) {
  Point2 a1(1, 1);
  Point2 b1(2, 1);
  Point2 c1(2, 2);
  Point2 d1(1, 2);
  EXPECT_FALSE(Quad2(a1, b1, c1, d1).IsCounterClockwise());
  EXPECT_FALSE(Quad2(b1, c1, d1, a1).IsCounterClockwise());
  EXPECT_TRUE(Quad2(a1, d1, c1, b1).IsCounterClockwise());
  EXPECT_TRUE(Quad2(c1, b1, a1, d1).IsCounterClockwise());

  // Slightly more complicated quads should work just as easily.
  Point2 a2(1.3f, 1.4f);
  Point2 b2(-0.7f, 4.9f);
  Point2 c2(1.8f, 6.2f);
  Point2 d2(2.1f, 1.6f);
  EXPECT_TRUE(Quad2(a2, b2, c2, d2).IsCounterClockwise());
  EXPECT_TRUE(Quad2(b2, c2, d2, a2).IsCounterClockwise());
  EXPECT_FALSE(Quad2(a2, d2, c2, b2).IsCounterClockwise());
  EXPECT_FALSE(Quad2(c2, b2, a2, d2).IsCounterClockwise());

  // Quads with 3 collinear points should work correctly, too.
  Point2 a3(0, 0);
  Point2 b3(1, 0);
  Point2 c3(2, 0);
  Point2 d3(1, 1);
  EXPECT_FALSE(Quad2(a3, b3, c3, d3).IsCounterClockwise());
  EXPECT_FALSE(Quad2(b3, c3, d3, a3).IsCounterClockwise());
  EXPECT_TRUE(Quad2(a3, d3, c3, b3).IsCounterClockwise());
  // The next expectation in particular would fail for an implementation
  // that incorrectly uses only a cross product of the first 3 vertices.
  EXPECT_TRUE(Quad2(c3, b3, a3, d3).IsCounterClockwise());

  // Non-convex quads should work correctly, too.
  Point2 a4(0, 0);
  Point2 b4(1, 1);
  Point2 c4(2, 0);
  Point2 d4(1, 3);
  EXPECT_FALSE(Quad2(a4, b4, c4, d4).IsCounterClockwise());
  EXPECT_FALSE(Quad2(b4, c4, d4, a4).IsCounterClockwise());
  EXPECT_TRUE(Quad2(a4, d4, c4, b4).IsCounterClockwise());
  EXPECT_TRUE(Quad2(c4, b4, a4, d4).IsCounterClockwise());

  // A quad with huge coordinates should not fail this check due to
  // single-precision overflow.
  Point2 a5(1e30f, 1e30f);
  Point2 b5(1e35f, 1e30f);
  Point2 c5(1e35f, 1e35f);
  Point2 d5(1e30f, 1e35f);
  EXPECT_FALSE(Quad2(a5, b5, c5, d5).IsCounterClockwise());
  EXPECT_FALSE(Quad2(b5, c5, d5, a5).IsCounterClockwise());
  EXPECT_TRUE(Quad2(a5, d5, c5, b5).IsCounterClockwise());
  EXPECT_TRUE(Quad2(c5, b5, a5, d5).IsCounterClockwise());
}

TEST(QuadTest, ContainsPoint) {
  Point2 a(1.3f, 1.4f);
  Point2 b(-0.8f, 4.4f);
  Point2 c(1.8f, 6.1f);
  Point2 d(2.1f, 1.6f);

  Vector2 epsilon_x(2 * Limits<float>::Epsilon, 0);
  Vector2 epsilon_y(0, 2 * Limits<float>::Epsilon);

  Vector2 ac_center = c - a;
  ac_center *= 0.5f;
  Vector2 bd_center = d - b;
  bd_center *= 0.5f;

  EXPECT_TRUE(Quad2(a, b, c, d).Contains(a + ac_center));
  EXPECT_TRUE(Quad2(a, b, c, d).Contains(b + bd_center));
  EXPECT_TRUE(Quad2(a, b, c, d).Contains(c - ac_center));
  EXPECT_TRUE(Quad2(a, b, c, d).Contains(d - bd_center));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(a - ac_center));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(b - bd_center));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(c + ac_center));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(d + bd_center));

  EXPECT_TRUE(Quad2(a, b, c, d).Contains(a));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(a - epsilon_x));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(a - epsilon_y));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(a + epsilon_x));
  EXPECT_TRUE(Quad2(a, b, c, d).Contains(a + epsilon_y));

  EXPECT_TRUE(Quad2(a, b, c, d).Contains(b));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(b - epsilon_x));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(b - epsilon_y));
  EXPECT_TRUE(Quad2(a, b, c, d).Contains(b + epsilon_x));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(b + epsilon_y));

  EXPECT_TRUE(Quad2(a, b, c, d).Contains(c));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(c - epsilon_x));
  EXPECT_TRUE(Quad2(a, b, c, d).Contains(c - epsilon_y));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(c + epsilon_x));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(c + epsilon_y));

  EXPECT_TRUE(Quad2(a, b, c, d).Contains(d));
  EXPECT_TRUE(Quad2(a, b, c, d).Contains(d - epsilon_x));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(d - epsilon_y));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(d + epsilon_x));
  EXPECT_FALSE(Quad2(a, b, c, d).Contains(d + epsilon_y));

  // Test a simple square.
  Point2 s1(-1, -1);
  Point2 s2(1, -1);
  Point2 s3(1, 1);
  Point2 s4(-1, 1);
  // Top edge.
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(-1.1f, -1.0f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(-1.0f, -1.0f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(0.0f, -1.0f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(1.0f, -1.0f)));
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(1.1f, -1.0f)));
  // Bottom edge.
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(-1.1f, 1.0f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(-1.0f, 1.0f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(0.0f, 1.0f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(1.0f, 1.0f)));
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(1.1f, 1.0f)));
  // Left edge.
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(-1.0f, -1.1f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(-1.0f, -1.0f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(-1.0f, 0.0f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(-1.0f, 1.0f)));
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(-1.0f, 1.1f)));
  // Right edge.
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(1.0f, -1.1f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(1.0f, -1.0f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(1.0f, 0.0f)));
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(1.0f, 1.0f)));
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(1.0f, 1.1f)));
  // Centered inside.
  EXPECT_TRUE(Quad2(s1, s2, s3, s4).Contains(Point2(0, 0)));
  // Centered outside.
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(-1.1f, 0)));
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(1.1f, 0)));
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(0, -1.1f)));
  EXPECT_FALSE(Quad2(s1, s2, s3, s4).Contains(Point2(0, 1.1f)));
}

TEST(QuadTest, Scale) {
  Point2 a(1.3f, 1.4f);
  Point2 b(-0.8f, 4.4f);
  Point2 c(1.8f, 6.1f);
  Point2 d(2.1f, 1.6f);
  Quad2 q1(a, b, c, d);
  q1 *= 1.5f;

  Point2 a_scaled = a * 1.5f;
  Point2 b_scaled = b * 1.5f;
  Point2 c_scaled = c * 1.5f;
  Point2 d_scaled = d * 1.5f;
  EXPECT_EQ(q1, Quad2(a_scaled, b_scaled, c_scaled, d_scaled));

  Quad2 q2;
  q2 *= 1.5f;
  EXPECT_EQ(q2, q2);
}

} // namespace stp
