// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Geometry/Vector2.h"

#include "Base/Math/Math.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(Vector2Test, ConversionToFloat) {
  IntVector2 i(3, 4);
  Vector2 f = static_cast<Vector2>(i);
  EXPECT_EQ(Vector2(3.0f, 4.0f), f);
}

TEST(Vector2Test, IsZero) {
  IntVector2 int_zero(0, 0);
  IntVector2 int_nonzero(2, -2);
  Vector2 float_zero(0, 0);
  Vector2 float_nonzero(0.1f, -0.1f);

  EXPECT_TRUE(int_zero.IsZero());
  EXPECT_FALSE(int_nonzero.IsZero());
  EXPECT_TRUE(float_zero.IsZero());
  EXPECT_FALSE(float_nonzero.IsZero());
}

TEST(Vector2Test, Add) {
  IntVector2 i1(3, 5);
  IntVector2 i2(4, -1);

  const struct {
    IntVector2 expected;
    IntVector2 actual;
  } int_cases[] = {
    { IntVector2(3, 5), i1 + IntVector2() },
    { IntVector2(3 + 4, 5 - 1), i1 + i2 },
    { IntVector2(3 - 4, 5 + 1), i1 - i2 }
  };

  for (const auto& item : int_cases)
    EXPECT_EQ(item.expected, item.actual);

  Vector2 f1(3.1f, 5.1f);
  Vector2 f2(4.3f, -1.3f);

  const struct {
    Vector2 expected;
    Vector2 actual;
  } float_cases[] = {
    { Vector2(3.1F, 5.1F), f1 + Vector2() },
    { Vector2(3.1f + 4.3f, 5.1f - 1.3f), f1 + f2 },
    { Vector2(3.1f - 4.3f, 5.1f + 1.3f), f1 - f2 }
  };

  for (const auto& item : float_cases)
    EXPECT_EQ(item.expected, item.actual);
}

TEST(Vector2Test, Negative) {
  const struct {
    IntVector2 expected;
    IntVector2 actual;
  } int_cases[] = {
    { IntVector2(0, 0), -IntVector2(0, 0) },
    { IntVector2(-3, -3), -IntVector2(3, 3) },
    { IntVector2(3, 3), -IntVector2(-3, -3) },
    { IntVector2(-3, 3), -IntVector2(3, -3) },
    { IntVector2(3, -3), -IntVector2(-3, 3) }
  };

  for (const auto& item : int_cases) {
    EXPECT_EQ(item.expected, item.actual);
  }

  const struct {
    Vector2 expected;
    Vector2 actual;
  } float_cases[] = {
    { Vector2(0, 0), Vector2(-IntVector2(0, 0)) },
    { Vector2(-0.3f, -0.3f), -Vector2(0.3f, 0.3f) },
    { Vector2(0.3f, 0.3f), -Vector2(-0.3f, -0.3f) },
    { Vector2(-0.3f, 0.3f), -Vector2(0.3f, -0.3f) },
    { Vector2(0.3f, -0.3f), -Vector2(-0.3f, 0.3f) }
  };

  for (const auto& item : float_cases) {
    EXPECT_EQ(item.expected, item.actual);
  }
}

TEST(Vector2Test, Scale) {
  float double_values[][4] = {
    { 4.5f, 1.2f, 3.3f, 5.6f },
    { 4.5f, -1.2f, 3.3f, 5.6f },
    { 4.5f, 1.2f, 3.3f, -5.6f },
    { 4.5f, 1.2f, -3.3f, -5.6f },
    { -4.5f, 1.2f, 3.3f, 5.6f },
    { -4.5f, 1.2f, 0, 5.6f },
    { -4.5f, 1.2f, 3.3f, 0 },
    { 4.5f, 0, 3.3f, 5.6f },
    { 0, 1.2f, 3.3f, 5.6f }
  };

  for (int i = 0; i < isizeofArray(double_values); ++i) {
    Vector2 v(double_values[i][0], double_values[i][1]);
    v = v.GetScaled(double_values[i][2], double_values[i][3]);
    EXPECT_EQ(v.x, double_values[i][0] * double_values[i][2]);
    EXPECT_EQ(v.y, double_values[i][1] * double_values[i][3]);
  }

  float single_values[][3] = {
    { 4.5f, 1.2f, 3.3f },
    { 4.5f, -1.2f, 3.3f },
    { 4.5f, 1.2f, 3.3f },
    { 4.5f, 1.2f, -3.3f },
    { -4.5f, 1.2f, 3.3f },
    { -4.5f, 1.2f, 0 },
    { -4.5f, 1.2f, 3.3f },
    { 4.5f, 0, 3.3f },
    { 0, 1.2f, 3.3f }
  };

  for (int i = 0; i < isizeofArray(single_values); ++i) {
    Vector2 v(single_values[i][0], single_values[i][1]);
    v *= single_values[i][2];
    EXPECT_EQ(v.x, single_values[i][0] * single_values[i][2]);
    EXPECT_EQ(v.y, single_values[i][1] * single_values[i][2]);

    Vector2 v2 = Vector2(double_values[i][0], double_values[i][1]) * double_values[i][2];
    EXPECT_EQ(single_values[i][0] * single_values[i][2], v2.x);
    EXPECT_EQ(single_values[i][1] * single_values[i][2], v2.y);
  }
}

TEST(Vector2Test, GetLength) {
  int int_values[][2] = {
    { 0, 0 },
    { 10, 20 },
    { 20, 10 },
    { -10, -20 },
    { -20, 10 },
    { 10, -20 },
  };

  for (const auto& pair : int_values) {
    int v0 = pair[0];
    int v1 = pair[1];
    double length_squared = static_cast<double>(v0) * v0 + static_cast<double>(v1) * v1;
    double length = mathSqrt(length_squared);
    IntVector2 vector(v0, v1);
    EXPECT_EQ(length_squared, vector.GetLengthSquared());
    EXPECT_FLOAT_EQ(length, vector.GetLength());
  }

  float float_values[][2] = {
    { 0, 0 },
    { 10.5f, 20.5f },
    { 20.5f, 10.5f },
    { -10.5f, -20.5f },
    { -20.5f, 10.5f },
    { 10.5f, -20.5f },
    // A large vector that fails if the Length function doesn't use
    // double precision internally.
    { 1236278317862780234892374893213178027.12122348904204230f,
      335890352589839028212313231225425134332.38123f },
  };

  for (const auto& pair : float_values) {
    double v0 = pair[0];
    double v1 = pair[1];
    double length_squared = static_cast<double>(v0) * v0 + static_cast<double>(v1) * v1;
    double length = mathSqrt(length_squared);
    Vector2 vector(v0, v1);
    EXPECT_DOUBLE_EQ(length_squared, vector.GetLengthSquared());
    EXPECT_FLOAT_EQ(length, vector.GetLength());
  }
}

TEST(Vector2Test, ClampInt) {
  IntVector2 a;

  a = IntVector2(3, 5);
  EXPECT_EQ(IntVector2(3, 5), a);
  a = max(a, IntVector2(2, 4));
  EXPECT_EQ(IntVector2(3, 5), a);
  a = max(a, IntVector2(3, 5));
  EXPECT_EQ(IntVector2(3, 5), a);
  a = max(a, IntVector2(4, 2));
  EXPECT_EQ(IntVector2(4, 5), a);
  a = max(a, IntVector2(8, 10));
  EXPECT_EQ(IntVector2(8, 10), a);

  a = min(a, IntVector2(9, 11));
  EXPECT_EQ(IntVector2(8, 10), a);
  a = min(a, IntVector2(8, 10));
  EXPECT_EQ(IntVector2(8, 10), a);
  a = min(a, IntVector2(11, 9));
  EXPECT_EQ(IntVector2(8, 9), a);
  a = min(a, IntVector2(7, 11));
  EXPECT_EQ(IntVector2(7, 9), a);
  a = min(a, IntVector2(3, 5));
  EXPECT_EQ(IntVector2(3, 5), a);
}

TEST(Vector2Test, Clamp) {
  Vector2 a;

  a = Vector2(3.5f, 5.5f);
  EXPECT_EQ(Vector2(3.5f, 5.5f), a);
  a = max(a, Vector2(2.5f, 4.5f));
  EXPECT_EQ(Vector2(3.5f, 5.5f), a);
  a = max(a, Vector2(3.5f, 5.5f));
  EXPECT_EQ(Vector2(3.5f, 5.5f), a);
  a = max(a, Vector2(4.5f, 2.5f));
  EXPECT_EQ(Vector2(4.5f, 5.5f), a);
  a = max(a, Vector2(8.5f, 10.5f));
  EXPECT_EQ(Vector2(8.5f, 10.5f), a);

  a = min(a, Vector2(9.5f, 11.5f));
  EXPECT_EQ(Vector2(8.5f, 10.5f), a);
  a = min(a, Vector2(8.5f, 10.5f));
  EXPECT_EQ(Vector2(8.5f, 10.5f), a);
  a = min(a, Vector2(11.5f, 9.5f));
  EXPECT_EQ(Vector2(8.5f, 9.5f), a);
  a = min(a, Vector2(7.5f, 11.5f));
  EXPECT_EQ(Vector2(7.5f, 9.5f), a);
  a = min(a, Vector2(3.5f, 5.5f));
  EXPECT_EQ(Vector2(3.5f, 5.5f), a);
}

} // namespace stp
