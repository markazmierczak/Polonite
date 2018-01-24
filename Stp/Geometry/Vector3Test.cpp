// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Geometry/Vector3.h"

#include "Base/Math/Math.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(Vector3Test, IsZero) {
  Vector3 float_zero(0, 0, 0);
  Vector3 float_nonzero(0.1f, -0.1f, 0.1f);

  EXPECT_TRUE(float_zero.IsZero());
  EXPECT_FALSE(float_nonzero.IsZero());
}

TEST(Vector3Test, Add) {
  Vector3 f1(3.1f, 5.1f, 2.7f);
  Vector3 f2(4.3f, -1.3f, 8.1f);

  const struct {
    Vector3 expected;
    Vector3 actual;
  } float_tests[] = {
    { Vector3(3.1F, 5.1F, 2.7f), f1 + Vector3() },
    { Vector3(3.1f + 4.3f, 5.1f - 1.3f, 2.7f + 8.1f), f1 + f2 },
    { Vector3(3.1f - 4.3f, 5.1f + 1.3f, 2.7f - 8.1f), f1 - f2 }
  };

  for (const auto& item : float_tests)
    EXPECT_EQ(item.expected, item.actual);
}

TEST(Vector3Test, Negative) {
  const struct {
    Vector3 expected;
    Vector3 actual;
  } float_tests[] = {
    { Vector3(-0.0f, -0.0f, -0.0f), -Vector3(0, 0, 0) },
    { Vector3(-0.3f, -0.3f, -0.3f), -Vector3(0.3f, 0.3f, 0.3f) },
    { Vector3(0.3f, 0.3f, 0.3f), -Vector3(-0.3f, -0.3f, -0.3f) },
    { Vector3(-0.3f, 0.3f, -0.3f), -Vector3(0.3f, -0.3f, 0.3f) },
    { Vector3(0.3f, -0.3f, -0.3f), -Vector3(-0.3f, 0.3f, 0.3f) },
    { Vector3(-0.3f, -0.3f, 0.3f), -Vector3(0.3f, 0.3f, -0.3f) }
  };

  for (const auto& item : float_tests)
    EXPECT_EQ(item.expected, item.actual);
}

TEST(Vector3Test, Scale) {
  float triple_values[][6] = {
    { 4.5f, 1.2f, 1.8f, 3.3f, 5.6f, 4.2f },
    { 4.5f, -1.2f, -1.8f, 3.3f, 5.6f, 4.2f },
    { 4.5f, 1.2f, -1.8f, 3.3f, 5.6f, 4.2f },
    { 4.5f, -1.2f -1.8f, 3.3f, 5.6f, 4.2f },

    { 4.5f, 1.2f, 1.8f, 3.3f, -5.6f, -4.2f },
    { 4.5f, 1.2f, 1.8f, -3.3f, -5.6f, -4.2f },
    { 4.5f, 1.2f, -1.8f, 3.3f, -5.6f, -4.2f },
    { 4.5f, 1.2f, -1.8f, -3.3f, -5.6f, -4.2f },

    { -4.5f, 1.2f, 1.8f, 3.3f, 5.6f, 4.2f },
    { -4.5f, 1.2f, 1.8f, 0, 5.6f, 4.2f },
    { -4.5f, 1.2f, -1.8f, 3.3f, 5.6f, 4.2f },
    { -4.5f, 1.2f, -1.8f, 0, 5.6f, 4.2f },

    { -4.5f, 1.2f, 1.8f, 3.3f, 0, 4.2f },
    { 4.5f, 0, 1.8f, 3.3f, 5.6f, 4.2f },
    { -4.5f, 1.2f, -1.8f, 3.3f, 0, 4.2f },
    { 4.5f, 0, -1.8f, 3.3f, 5.6f, 4.2f },
    { -4.5f, 1.2f, 1.8f, 3.3f, 5.6f, 0 },
    { -4.5f, 1.2f, -1.8f, 3.3f, 5.6f, 0 },

    { 0, 1.2f, 0, 3.3f, 5.6f, 4.2f },
    { 0, 1.2f, 1.8f, 3.3f, 5.6f, 4.2f }
  };

  for (int i = 0; i < ArraySizeOf(triple_values); ++i) {
    Vector3 v(
        triple_values[i][0],
        triple_values[i][1],
        triple_values[i][2]);

    v.Scale(triple_values[i][3], triple_values[i][4], triple_values[i][5]);
    EXPECT_EQ(triple_values[i][0] * triple_values[i][3], v.x);
    EXPECT_EQ(triple_values[i][1] * triple_values[i][4], v.y);
    EXPECT_EQ(triple_values[i][2] * triple_values[i][5], v.z);

    Vector3 v2 = Vector3(triple_values[i][0], triple_values[i][1], triple_values[i][2])
        .GetScaled(triple_values[i][3], triple_values[i][4], triple_values[i][5]);

    EXPECT_EQ(triple_values[i][0] * triple_values[i][3], v2.x);
    EXPECT_EQ(triple_values[i][1] * triple_values[i][4], v2.y);
    EXPECT_EQ(triple_values[i][2] * triple_values[i][5], v2.z);
  }

  float single_values[][4] = {
    { 4.5f, 1.2f, 1.8f, 3.3f },
    { 4.5f, -1.2f, 1.8f, 3.3f },
    { 4.5f, 1.2f, -1.8f, 3.3f },
    { 4.5f, -1.2f, -1.8f, 3.3f },
    { -4.5f, 1.2f, 3.3f },
    { -4.5f, 1.2f, 0 },
    { -4.5f, 1.2f, 1.8f, 3.3f },
    { -4.5f, 1.2f, 1.8f, 0 },
    { 4.5f, 0, 1.8f, 3.3f },
    { 0, 1.2f, 1.8f, 3.3f },
    { 4.5f, 0, 1.8f, 3.3f },
    { 0, 1.2f, 1.8f, 3.3f },
    { 4.5f, 1.2f, 0, 3.3f },
    { 4.5f, 1.2f, 0, 3.3f }
  };

  for (int i = 0; i < ArraySizeOf(single_values); ++i) {
    Vector3 v(
        single_values[i][0],
        single_values[i][1],
        single_values[i][2]);

    v *= single_values[i][3];
    EXPECT_EQ(single_values[i][0] * single_values[i][3], v.x);
    EXPECT_EQ(single_values[i][1] * single_values[i][3], v.y);
    EXPECT_EQ(single_values[i][2] * single_values[i][3], v.z);

    Vector3 v2 = Vector3(single_values[i][0], single_values[i][1], single_values[i][2]) *
        single_values[i][3];
    EXPECT_EQ(single_values[i][0] * single_values[i][3], v2.x);
    EXPECT_EQ(single_values[i][1] * single_values[i][3], v2.y);
    EXPECT_EQ(single_values[i][2] * single_values[i][3], v2.z);
  }
}

TEST(Vector3Test, GetLength) {
  float float_values[][3] = {
    { 0, 0, 0 },
    { 10.5f, 20.5f, 8.5f },
    { 20.5f, 10.5f, 8.5f },
    { 8.5f, 20.5f, 10.5f },
    { 10.5f, 8.5f, 20.5f },
    { -10.5f, -20.5f, -8.5f },
    { -20.5f, 10.5f, -8.5f },
    { -8.5f, -20.5f, -10.5f },
    { -10.5f, -8.5f, -20.5f },
    { 10.5f, -20.5f, 8.5f },
    { -10.5f, 20.5f, 8.5f },
    { 10.5f, -20.5f, -8.5f },
    { -10.5f, 20.5f, -8.5f },
    // A large vector that fails if the Length function doesn't use
    // double precision internally.
    { 1236278317862780234892374893213178027.12122348904204230f,
      335890352589839028212313231225425134332.38123f,
      27861786423846742743236423478236784678.236713617231f }
  };

  for (int i = 0; i < ArraySizeOf(float_values); ++i) {
    double v0 = float_values[i][0];
    double v1 = float_values[i][1];
    double v2 = float_values[i][2];
    double length_squared =
        static_cast<double>(v0) * v0 +
        static_cast<double>(v1) * v1 +
        static_cast<double>(v2) * v2;
    double length = Sqrt(length_squared);
    Vector3 vector(v0, v1, v2);
    EXPECT_DOUBLE_EQ(length_squared, vector.GetLengthSquared());
    EXPECT_DOUBLE_EQ(length, vector.GetLength());
  }
}

TEST(Vector3Test, Normalize) {
  EXPECT_TRUE(Vector3(1, 0, 0).IsNormalized());
  EXPECT_FALSE(Vector3(1, 1, 1).IsNormalized());
  EXPECT_FALSE(Vector3(0, 1, 2).IsNormalized());
  EXPECT_TRUE(Vector3(1, 2, 3).GetNormalizedOrThis().IsNormalized());
}

TEST(Vector3Test, DotProduct) {
  const struct {
    float expected;
    Vector3 input1;
    Vector3 input2;
  } tests[] = {
    { 0, Vector3(1, 0, 0), Vector3(0, 1, 1) },
    { 0, Vector3(0, 1, 0), Vector3(1, 0, 1) },
    { 0, Vector3(0, 0, 1), Vector3(1, 1, 0) },

    { 3, Vector3(1, 1, 1), Vector3(1, 1, 1) },

    { 1.2f, Vector3(1.2f, -1.2f, 1.2f), Vector3(1, 1, 1) },
    { 1.2f, Vector3(1, 1, 1), Vector3(1.2f, -1.2f, 1.2f) },

    { 38.72f,
      Vector3(1.1f, 2.2f, 3.3f), Vector3(4.4f, 5.5f, 6.6f) }
  };

  for (const auto& test : tests) {
    float actual = DotProduct(test.input1, test.input2);
    EXPECT_EQ(test.expected, actual);
  }
}

TEST(Vector3Test, CrossProduct) {
  const struct {
    Vector3 expected;
    Vector3 input1;
    Vector3 input2;
  } tests[] = {
    { Vector3(), Vector3(), Vector3(1, 1, 1) },
    { Vector3(), Vector3(1, 1, 1), Vector3() },
    { Vector3(), Vector3(1, 1, 1), Vector3(1, 1, 1) },
    { Vector3(),
      Vector3(1.6f, 10.6f, -10.6f),
      Vector3(1.6f, 10.6f, -10.6f) },

    { Vector3(1, -1, 0), Vector3(1, 1, 1), Vector3(0, 0, 1) },
    { Vector3(-1, 0, 1), Vector3(1, 1, 1), Vector3(0, 1, 0) },
    { Vector3(0, 1, -1), Vector3(1, 1, 1), Vector3(1, 0, 0) },

    { Vector3(-1, 1, 0), Vector3(0, 0, 1), Vector3(1, 1, 1) },
    { Vector3(1, 0, -1), Vector3(0, 1, 0), Vector3(1, 1, 1) },
    { Vector3(0, -1, 1), Vector3(1, 0, 0), Vector3(1, 1, 1) }
  };

  for (const auto& test : tests) {
    Vector3 actual = CrossProduct(test.input1, test.input2);
    EXPECT_EQ(test.expected, actual);
  }
}

TEST(Vector3Test, Clamp) {
  Vector3 a;

  a = Vector3(3.5f, 5.5f, 7.5f);
  EXPECT_EQ(Vector3(3.5f, 5.5f, 7.5f), a);
  a = Max(a, Vector3(2, 4.5f, 6.5f));
  EXPECT_EQ(Vector3(3.5f, 5.5f, 7.5f), a);
  a = Max(a, Vector3(3.5f, 5.5f, 7.5f));
  EXPECT_EQ(Vector3(3.5f, 5.5f, 7.5f), a);
  a = Max(a, Vector3(4.5f, 2, 6.5f));
  EXPECT_EQ(Vector3(4.5f, 5.5f, 7.5f), a);
  a = Max(a, Vector3(3.5f, 6.5f, 6.5f));
  EXPECT_EQ(Vector3(4.5f, 6.5f, 7.5f), a);
  a = Max(a, Vector3(3.5f, 5.5f, 8.5f));
  EXPECT_EQ(Vector3(4.5f, 6.5f, 8.5f), a);
  a = Max(a, Vector3(8.5f, 10.5f, 12.5f));
  EXPECT_EQ(Vector3(8.5f, 10.5f, 12.5f), a);

  a = Min(a, Vector3(9.5f, 11.5f, 13.5f));
  EXPECT_EQ(Vector3(8.5f, 10.5f, 12.5f), a);
  a = Min(a, Vector3(8.5f, 10.5f, 12.5f));
  EXPECT_EQ(Vector3(8.5f, 10.5f, 12.5f), a);
  a = Min(a, Vector3(7.5f, 11.5f, 13.5f));
  EXPECT_EQ(Vector3(7.5f, 10.5f, 12.5f), a);
  a = Min(a, Vector3(9.5f, 9.5f, 13.5f));
  EXPECT_EQ(Vector3(7.5f, 9.5f, 12.5f), a);
  a = Min(a, Vector3(9.5f, 11.5f, 11.5f));
  EXPECT_EQ(Vector3(7.5f, 9.5f, 11.5f), a);
  a = Min(a, Vector3(3.5f, 5.5f, 7.5f));
  EXPECT_EQ(Vector3(3.5f, 5.5f, 7.5f), a);
}

} // namespace stp
