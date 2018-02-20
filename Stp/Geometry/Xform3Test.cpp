// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Geometry/Xform3.h"

#include "Base/Test/GTest.h"
#include "Geometry/Quad2.h"

namespace stp {
namespace {

#define EXPECT_ROW1_EQ(a, b, c, d, transform) \
  EXPECT_FLOAT_EQ((a), (transform).get(0, 0)); \
  EXPECT_FLOAT_EQ((b), (transform).get(0, 1)); \
  EXPECT_FLOAT_EQ((c), (transform).get(0, 2)); \
  EXPECT_FLOAT_EQ((d), (transform).get(0, 3));

#define EXPECT_ROW2_EQ(a, b, c, d, transform) \
  EXPECT_FLOAT_EQ((a), (transform).get(1, 0)); \
  EXPECT_FLOAT_EQ((b), (transform).get(1, 1)); \
  EXPECT_FLOAT_EQ((c), (transform).get(1, 2)); \
  EXPECT_FLOAT_EQ((d), (transform).get(1, 3));

#define EXPECT_ROW3_EQ(a, b, c, d, transform) \
  EXPECT_FLOAT_EQ((a), (transform).get(2, 0)); \
  EXPECT_FLOAT_EQ((b), (transform).get(2, 1)); \
  EXPECT_FLOAT_EQ((c), (transform).get(2, 2)); \
  EXPECT_FLOAT_EQ((d), (transform).get(2, 3));

#define EXPECT_ROW4_EQ(a, b, c, d, transform) \
  EXPECT_FLOAT_EQ((a), (transform).get(3, 0)); \
  EXPECT_FLOAT_EQ((b), (transform).get(3, 1)); \
  EXPECT_FLOAT_EQ((c), (transform).get(3, 2)); \
  EXPECT_FLOAT_EQ((d), (transform).get(3, 3)); \

// Checking float values for equality close to zero is not robust using
// EXPECT_FLOAT_EQ (see gtest documentation). So, to verify rotation matrices,
// we must use a looser absolute error threshold in some places.
#define EXPECT_ROW1_NEAR(a, b, c, d, transform, error_threshold) \
  EXPECT_NEAR((a), (transform).get(0, 0), (error_threshold)); \
  EXPECT_NEAR((b), (transform).get(0, 1), (error_threshold)); \
  EXPECT_NEAR((c), (transform).get(0, 2), (error_threshold)); \
  EXPECT_NEAR((d), (transform).get(0, 3), (error_threshold));

#define EXPECT_ROW2_NEAR(a, b, c, d, transform, error_threshold) \
  EXPECT_NEAR((a), (transform).get(1, 0), (error_threshold)); \
  EXPECT_NEAR((b), (transform).get(1, 1), (error_threshold)); \
  EXPECT_NEAR((c), (transform).get(1, 2), (error_threshold)); \
  EXPECT_NEAR((d), (transform).get(1, 3), (error_threshold));

#define EXPECT_ROW3_NEAR(a, b, c, d, transform, error_threshold) \
  EXPECT_NEAR((a), (transform).get(2, 0), (error_threshold)); \
  EXPECT_NEAR((b), (transform).get(2, 1), (error_threshold)); \
  EXPECT_NEAR((c), (transform).get(2, 2), (error_threshold)); \
  EXPECT_NEAR((d), (transform).get(2, 3), (error_threshold));

bool PointsAreNearlyEqual(const Point3& lhs, const Point3& rhs) {
  double epsilon = 1E-4;
  return (lhs - rhs).GetLengthSquared() < epsilon;
}

bool MatricesAreNearlyEqual(const Xform3& lhs, const Xform3& rhs) {
  return isNear(lhs, rhs, 1E-3f);
}

void InitializeTestMatrix(Xform3& transform) {
  transform.set(0, 0, 10.f);
  transform.set(1, 0, 11.f);
  transform.set(2, 0, 12.f);
  transform.set(3, 0, 13.f);
  transform.set(0, 1, 14.f);
  transform.set(1, 1, 15.f);
  transform.set(2, 1, 16.f);
  transform.set(3, 1, 17.f);
  transform.set(0, 2, 18.f);
  transform.set(1, 2, 19.f);
  transform.set(2, 2, 20.f);
  transform.set(3, 2, 21.f);
  transform.set(0, 3, 22.f);
  transform.set(1, 3, 23.f);
  transform.set(2, 3, 24.f);
  transform.set(3, 3, 25.f);

  // Sanity check
  EXPECT_ROW1_EQ(10.0f, 14.0f, 18.0f, 22.0f, transform);
  EXPECT_ROW2_EQ(11.0f, 15.0f, 19.0f, 23.0f, transform);
  EXPECT_ROW3_EQ(12.0f, 16.0f, 20.0f, 24.0f, transform);
  EXPECT_ROW4_EQ(13.0f, 17.0f, 21.0f, 25.0f, transform);
}

void InitializeTestMatrix2(Xform3& transform) {
  transform.set(0, 0, 30.f);
  transform.set(1, 0, 31.f);
  transform.set(2, 0, 32.f);
  transform.set(3, 0, 33.f);
  transform.set(0, 1, 34.f);
  transform.set(1, 1, 35.f);
  transform.set(2, 1, 36.f);
  transform.set(3, 1, 37.f);
  transform.set(0, 2, 38.f);
  transform.set(1, 2, 39.f);
  transform.set(2, 2, 40.f);
  transform.set(3, 2, 41.f);
  transform.set(0, 3, 42.f);
  transform.set(1, 3, 43.f);
  transform.set(2, 3, 44.f);
  transform.set(3, 3, 45.f);

  // Sanity check
  EXPECT_ROW1_EQ(30.0f, 34.0f, 38.0f, 42.0f, transform);
  EXPECT_ROW2_EQ(31.0f, 35.0f, 39.0f, 43.0f, transform);
  EXPECT_ROW3_EQ(32.0f, 36.0f, 40.0f, 44.0f, transform);
  EXPECT_ROW4_EQ(33.0f, 37.0f, 41.0f, 45.0f, transform);
}

constexpr float ApproxZero = Limits<float>::Epsilon;
constexpr float ApproxOne = 1 - ApproxZero;

void InitializeApproxIdentityMatrix(Xform3& transform) {
  transform.set(0, 0, ApproxOne);
  transform.set(0, 1, ApproxZero);
  transform.set(0, 2, ApproxZero);
  transform.set(0, 3, ApproxZero);

  transform.set(1, 0, ApproxZero);
  transform.set(1, 1, ApproxOne);
  transform.set(1, 2, ApproxZero);
  transform.set(1, 3, ApproxZero);

  transform.set(2, 0, ApproxZero);
  transform.set(2, 1, ApproxZero);
  transform.set(2, 2, ApproxOne);
  transform.set(2, 3, ApproxZero);

  transform.set(3, 0, ApproxZero);
  transform.set(3, 1, ApproxZero);
  transform.set(3, 2, ApproxZero);
  transform.set(3, 3, ApproxOne);
}

#ifdef SK_MSCALAR_IS_DOUBLE
#define ERROR_THRESHOLD 1e-14
#else
#define ERROR_THRESHOLD 1e-7
#endif
#define LOOSE_ERROR_THRESHOLD 1e-7

TEST(Xform3Test, Equality) {
  Xform3 lhs = Xform3::Identity();

  Xform3 rhs(Xform3::SkipInit);
  rhs.Set3x3(
      1, 2, 3,
      4, 5, 6,
      7, 8, 9);

  Xform3 interpolated = lhs;
  for (int i = 0; i <= 100; ++i) {
    for (int row = 0; row < 4; ++row) {
      for (int col = 0; col < 4; ++col) {
        float a = lhs.get(row, col);
        float b = rhs.get(row, col);
        float t = i / 100.0f;
        interpolated.set(row, col, a + (b - a) * t);
      }
    }
    if (i == 100) {
      EXPECT_TRUE(rhs == interpolated);
    } else {
      EXPECT_TRUE(rhs != interpolated);
    }
  }
  lhs.SetIdentity();
  rhs.SetIdentity();
  for (int i = 1; i < 100; ++i) {
    lhs.SetIdentity();
    rhs.SetIdentity();
    lhs.Translate2D(i, i);
    rhs.Translate2D(-i, -i);
    EXPECT_TRUE(lhs != rhs);
    rhs.Translate2D(2*i, 2*i);
    EXPECT_TRUE(lhs == rhs);
  }
}

TEST(Xform3Test, ConcatTranslate) {
  static const struct TestCase {
    int x1;
    int y1;
    float tx;
    float ty;
    int x2;
    int y2;
  } test_cases[] = {
    { 0, 0, 10.0f, 20.0f, 10, 20 },
    { 0, 0, -10.0f, -20.0f, 0, 0 },
    { 0, 0, -10.0f, -20.0f, -10, -20 },
    { 0, 0,
      Limits<float>::NaN,
      Limits<float>::NaN,
      10, 20 },
  };

  Xform3 xform = Xform3::Identity();
  for (const TestCase& value : test_cases) {
    Xform3 translation = Xform3::Identity();
    translation.Translate2D(value.tx, value.ty);
    xform = translation * xform;
    Point3 p1(value.x1, value.y1, 0);
    Point3 p2(value.x2, value.y2, 0);
    p1 = xform.MapPoint(p1);
    if (value.tx == value.tx &&
        value.ty == value.ty) {
      EXPECT_TRUE(PointsAreNearlyEqual(p1, p2));
    }
  }
}

TEST(Xform3Test, ConcatScale) {
  static const struct TestCase {
    int before;
    float scale;
    int after;
  } test_cases[] = {
    { 1, 10.0f, 10 },
    { 1, .1f, 1 },
    { 1, 100.0f, 100 },
    { 1, -1.0f, -100 },
    { 1, Limits<float>::NaN, 1 }
  };

  Xform3 xform = Xform3::Identity();
  for (const TestCase& value : test_cases) {
    Xform3 scale = Xform3::Identity();
    scale.Scale2D(value.scale, value.scale);
    xform = scale * xform;
    Point3 p1(value.before, value.before, 0);
    Point3 p2(value.after, value.after, 0);
    p1 = xform.MapPoint(p1);
    if (value.scale == value.scale) {
      EXPECT_TRUE(PointsAreNearlyEqual(p1, p2));
    }
  }
}

TEST(Xform3Test, ConcatRotate) {
  static const struct TestCase {
    int x1;
    int y1;
    float degrees;
    int x2;
    int y2;
  } test_cases[] = {
    { 1, 0, 90.0f, 0, 1 },
    { 1, 0, -90.0f, 1, 0 },
    { 1, 0, 90.0f, 0, 1 },
    { 1, 0, 360.0f, 0, 1 },
    { 1, 0, 0.0f, 0, 1 },
    { 1, 0, Limits<float>::NaN, 1, 0 }
  };

  Xform3 xform = Xform3::Identity();
  for (const TestCase& value : test_cases) {
    Xform3 rotation = Xform3::Identity();
    rotation.Rotate2D(Angle::DegreesToRadians(value.degrees));
    xform = rotation * xform;
    Point3 p1(value.x1, value.y1, 0);
    Point3 p2(value.x2, value.y2, 0);
    p1 = xform.MapPoint(p1);
    if (value.degrees == value.degrees) {
      EXPECT_TRUE(PointsAreNearlyEqual(p1, p2));
    }
  }
}

TEST(Xform3Test, SetTranslate) {
  static const struct TestCase {
    int x1; int y1;
    float tx; float ty;
    int x2; int y2;
  } test_cases[] = {
    { 0, 0, 10.0f, 20.0f, 10, 20 },
    { 10, 20, 10.0f, 20.0f, 20, 40 },
    { 10, 20, 0.0f, 0.0f, 10, 20 },
    { 0, 0,
      Limits<float>::NaN,
      Limits<float>::NaN,
      0, 0 }
  };

  for (const TestCase& value : test_cases) {
    for (int k = 0; k < 3; ++k) {
      Point3 p0, p1, p2;
      Xform3 xform = Xform3::Identity();
      switch (k) {
      case 0:
        p1 = Point3(value.x1, 0, 0);
        p2 = Point3(value.x2, 0, 0);
        xform.Translate2D(value.tx, 0.0);
        break;
      case 1:
        p1 = Point3(0, value.y1, 0);
        p2 = Point3(0, value.y2, 0);
        xform.Translate2D(0.0, value.ty);
        break;
      case 2:
        p1 = Point3(value.x1, value.y1, 0);
        p2 = Point3(value.x2, value.y2, 0);
        xform.Translate2D(value.tx, value.ty);
        break;
      }
      p0 = p1;
      p1 = xform.MapPoint(p0);
      if (value.tx == value.tx &&
          value.ty == value.ty) {
        EXPECT_TRUE(PointsAreNearlyEqual(p1, p2));
      }
    }
  }
}

TEST(Xform3Test, SetScale) {
  static const struct TestCase {
    int before;
    float s;
    int after;
  } test_cases[] = {
    { 1, 10.0f, 10 },
    { 1, 1.0f, 1 },
    { 1, 0.0f, 0 },
    { 0, 10.0f, 0 },
    { 1, Limits<float>::NaN, 0 },
  };

  for (const TestCase& value : test_cases) {
    for (int k = 0; k < 3; ++k) {
      Point3 p0, p1, p2;
      Xform3 xform = Xform3::Identity();
      switch (k) {
      case 0:
        p1 = Point3(value.before, 0, 0);
        p2 = Point3(value.after, 0, 0);
        xform.Scale2D(value.s, 1.0);
        break;
      case 1:
        p1 = Point3(0, value.before, 0);
        p2 = Point3(0, value.after, 0);
        xform.Scale2D(1.0, value.s);
        break;
      case 2:
        p1 = Point3(value.before, value.before, 0);
        p2 = Point3(value.after, value.after, 0);
        xform.Scale2D(value.s, value.s);
        break;
      }
      p0 = p1;
      p1 = xform.MapPoint(p0);
      if (value.s == value.s) {
        EXPECT_TRUE(PointsAreNearlyEqual(p1, p2));
      }
    }
  }
}

TEST(Xform3Test, SetRotate) {
  static const struct SetRotateCase {
    int x;
    int y;
    float degree;
    int xprime;
    int yprime;
  } set_rotate_cases[] = {
    { 100, 0, 90.0f, 0, 100 },
    { 0, 0, 90.0f, 0, 0 },
    { 0, 100, 90.0f, -100, 0 },
    { 0, 1, -90.0f, 1, 0 },
    { 100, 0, 0.0f, 100, 0 },
    { 0, 0, 0.0f, 0, 0 },
    { 0, 0, Limits<float>::NaN, 0, 0 },
    { 100, 0, 360.0f, 100, 0 }
  };

  for (const SetRotateCase& value : set_rotate_cases) {
    Point3 p0;
    Point3 p1(value.x, value.y, 0);
    Point3 p2(value.xprime, value.yprime, 0);
    p0 = p1;
    Xform3 xform = Xform3::Identity();
    xform.Rotate2D(Angle::DegreesToRadians(value.degree));
    // just want to make sure that we don't crash in the case of NaN.
    if (value.degree == value.degree) {
      p1 = xform.MapPoint(p0);
      EXPECT_TRUE(PointsAreNearlyEqual(p1, p2));
    }
  }
}

TEST(Xform3Test, MapPointWithExtremePerspective) {
  Point3 point(1.f, 1.f, 1.f);
  Xform3 perspective = Xform3::Identity();
  perspective.ApplyPerspectiveDepth(1.f);
  Point3 transformed = point;
  transformed = perspective.MapPoint(transformed);
  EXPECT_EQ(point, transformed);

  transformed = point;
  perspective.SetIdentity();
  perspective.ApplyPerspectiveDepth(1.1f);
  transformed = perspective.MapPoint(transformed);
  EXPECT_FLOAT_EQ(11.f, transformed.x);
  EXPECT_FLOAT_EQ(11.f, transformed.y);
  EXPECT_FLOAT_EQ(11.f, transformed.z);
}

TEST(Xform3Test, BlendTranslate) {
  Xform3 from = Xform3::Identity();
  for (int i = -5; i < 15; ++i) {
    Xform3 to = Xform3::Identity();
    to.Translate(1, 1, 1);
    double t = i / 9.0;
    Xform3 res(Xform3::SkipInit);
    ASSERT_TRUE(Trylerp(res, from, to, t));
    EXPECT_FLOAT_EQ(t, res.get(0, 3));
    EXPECT_FLOAT_EQ(t, res.get(1, 3));
    EXPECT_FLOAT_EQ(t, res.get(2, 3));
  }
}

TEST(Xform3Test, BlendRotate) {
  Vector3 axes[] = {
    Vector3(1, 0, 0),
    Vector3(0, 1, 0),
    Vector3(0, 0, 1),
    Vector3(1, 1, 1)
  };
  Xform3 from = Xform3::Identity();
  for (Vector3 axis : axes) {
    for (int i = -5; i < 15; ++i) {
      Xform3 to = Xform3::Identity();
      to.RotateAbout(axis, Angle::DegreesToRadians(90));
      double t = i / 9.0;
      Xform3 res(Xform3::SkipInit);
      ASSERT_TRUE(Trylerp(res, from, to, t));

      Xform3 expected = Xform3::Identity();
      expected.RotateAbout(axis, Angle::DegreesToRadians(90 * t));

      EXPECT_TRUE(MatricesAreNearlyEqual(expected, res));
    }
  }
}

TEST(Xform3Test, CanBlend180DegreeRotation) {
  Vector3 axes[] = {
    Vector3(1, 0, 0),
    Vector3(0, 1, 0),
    Vector3(0, 0, 1),
    Vector3(1, 1, 1)
  };
  Xform3 from = Xform3::Identity();
  for (Vector3 axis : axes) {
    for (int i = -5; i < 15; ++i) {
      Xform3 to = Xform3::Identity();
      to.RotateAbout(axis, Angle::DegreesToRadians(180.0));

      double t = i / 9.0;
      Xform3 res(Xform3::SkipInit);
      ASSERT_TRUE(Trylerp(res, from, to, t));

      // A 180 degree rotation is exactly opposite on the sphere, therefore
      // either great circle arc to it is equivalent (and numerical precision
      // will determine which is closer).  Test both directions.
      Xform3 expected1 = Xform3::Identity();
      expected1.RotateAbout(axis, Angle::DegreesToRadians(180.0 * t));
      Xform3 expected2 = Xform3::Identity();
      expected2.RotateAbout(axis, Angle::DegreesToRadians(-180.0 * t));

      EXPECT_TRUE(
          MatricesAreNearlyEqual(expected1, res) ||
          MatricesAreNearlyEqual(expected2, res));
    }
  }
}

TEST(Xform3Test, BlendScale) {
  Xform3 from = Xform3::Identity();
  for (int i = -5; i < 15; ++i) {
    Xform3 to = Xform3::Identity();
    to.Scale(5, 4, 3);
    double t = i / 9.0;
    Xform3 res(Xform3::SkipInit);
    ASSERT_TRUE(Trylerp(res, from, to, t));
    EXPECT_FLOAT_EQ(t * 4 + 1, res.get(0, 0));
    EXPECT_FLOAT_EQ(t * 3 + 1, res.get(1, 1));
    EXPECT_FLOAT_EQ(t * 2 + 1, res.get(2, 2));
  }
}

TEST(Xform3Test, BlendSkew) {
  Xform3 from = Xform3::Identity();
  for (int i = 0; i < 2; ++i) {
    Xform3 to = Xform3::Identity();
    to.Skew(10, 5);
    double t = i;
    Xform3 expected = Xform3::Identity();
    expected.Skew(t * 10, t * 5);
    Xform3 res(Xform3::SkipInit);
    ASSERT_TRUE(Trylerp(res, from, to, t));
    EXPECT_TRUE(MatricesAreNearlyEqual(expected, res));
  }
}

TEST(Xform3Test, ExtrapolateSkew) {
  Xform3 from = Xform3::Identity();
  for (int i = -1; i < 2; ++i) {
    Xform3 to = Xform3::Identity();
    to.Skew(20, 0);
    double t = i;
    Xform3 expected = Xform3::Identity();
    expected.Skew(t * 20, t * 0);
    Xform3 res(Xform3::SkipInit);
    ASSERT_TRUE(Trylerp(res, from, to, t));
    EXPECT_TRUE(MatricesAreNearlyEqual(expected, res));
  }
}

TEST(Xform3Test, BlendPerspective) {
  Xform3 from = Xform3::Identity();
  from.ApplyPerspectiveDepth(200);

  for (int i = -1; i < 3; ++i) {
    Xform3 to = Xform3::Identity();
    to.ApplyPerspectiveDepth(800);
    double t = i;
    double depth = 1.0 / lerp(1.0 / 200, 1.0 / 800, t);
    Xform3 expected = Xform3::Identity();
    expected.ApplyPerspectiveDepth(depth);
    Xform3 res(Xform3::SkipInit);
    ASSERT_TRUE(Trylerp(res, from, to, t));
    EXPECT_TRUE(MatricesAreNearlyEqual(expected, res));
  }
}

TEST(Xform3Test, BlendIdentity) {
  Xform3 from = Xform3::Identity();
  Xform3 to = Xform3::Identity();
  Xform3 res(Xform3::SkipInit);
  ASSERT_TRUE(Trylerp(res, from, to, 0.5));
  EXPECT_EQ(to, res);
}

TEST(Xform3Test, CannotBlendSingularMatrix) {
  Xform3 from = Xform3::Identity();
  Xform3 to = Xform3::Identity();
  to.set(3, 3, 0);
  Xform3 res(Xform3::SkipInit);
  EXPECT_FALSE(Trylerp(res, from, to, 0.5));
}

TEST(Xform3Test, VerifyBlendForTranslation) {
  Xform3 from = Xform3::Identity();
  from.Translate(100.0, 200.0, 100.0);

  Xform3 to = Xform3::Identity();

  to.Translate(200.0, 100.0, 300.0);
  Xform3 res(Xform3::SkipInit);
  ASSERT_TRUE(Trylerp(res, from, to, 0.0));
  EXPECT_EQ(from, res);

  to.SetIdentity();
  to.Translate(200.0, 100.0, 300.0);
  ASSERT_TRUE(Trylerp(res, from, to, 0.25));
  EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, 125.0f, res);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 175.0f, res);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 150.0f, res);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f,  1.0f,  res);

  to.SetIdentity();
  to.Translate(200.0, 100.0, 300.0);
  ASSERT_TRUE(Trylerp(res, from, to, 0.5));
  EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, 150.0f, res);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 150.0f, res);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 200.0f, res);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f,  1.0f,  res);

  to.SetIdentity();
  to.Translate(200.0, 100.0, 300.0);
  ASSERT_TRUE(Trylerp(res, from, to, 1.0));
  EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, 200.0f, res);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 100.0f, res);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 300.0f, res);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f,  1.0f,  res);
}

TEST(Xform3Test, VerifyBlendForScale) {
  Xform3 from = Xform3::Identity();
  from.Scale(100.0, 200.0, 100.0);

  Xform3 to = Xform3::Identity();

  to.Scale(200.0, 100.0, 300.0);
  Xform3 res(Xform3::SkipInit);
  ASSERT_TRUE(Trylerp(res, from, to, 0.0));
  EXPECT_EQ(from, res);

  to.SetIdentity();
  to.Scale(200.0, 100.0, 300.0);
  ASSERT_TRUE(Trylerp(res, from, to, 0.25));
  EXPECT_ROW1_EQ(125.0f, 0.0f,  0.0f,  0.0f, res);
  EXPECT_ROW2_EQ(0.0f,  175.0f, 0.0f,  0.0f, res);
  EXPECT_ROW3_EQ(0.0f,   0.0f, 150.0f, 0.0f, res);
  EXPECT_ROW4_EQ(0.0f,   0.0f,  0.0f,  1.0f, res);

  to.SetIdentity();
  to.Scale(200.0, 100.0, 300.0);
  ASSERT_TRUE(Trylerp(res, from, to, 0.5));
  EXPECT_ROW1_EQ(150.0f, 0.0f,  0.0f,  0.0f, res);
  EXPECT_ROW2_EQ(0.0f,  150.0f, 0.0f,  0.0f, res);
  EXPECT_ROW3_EQ(0.0f,   0.0f, 200.0f, 0.0f, res);
  EXPECT_ROW4_EQ(0.0f,   0.0f,  0.0f,  1.0f, res);

  to.SetIdentity();
  to.Scale(200.0, 100.0, 300.0);
  ASSERT_TRUE(Trylerp(res, from, to, 1.0));
  EXPECT_ROW1_EQ(200.0f, 0.0f,  0.0f,  0.0f, res);
  EXPECT_ROW2_EQ(0.0f,  100.0f, 0.0f,  0.0f, res);
  EXPECT_ROW3_EQ(0.0f,   0.0f, 300.0f, 0.0f, res);
  EXPECT_ROW4_EQ(0.0f,   0.0f,  0.0f,  1.0f, res);
}

TEST(Xform3Test, VerifyBlendForSkew) {
  // Along X axis only
  Xform3 from = Xform3::Identity();
  from.Skew(0.0, 0.0);

  Xform3 to = Xform3::Identity();

  to.Skew(45.0, 0.0);

  Xform3 res(Xform3::SkipInit);
  ASSERT_TRUE(Trylerp(res, from, to, 0.0));
  EXPECT_EQ(from, res);

  to.SetIdentity();
  to.Skew(45.0, 0.0);
  ASSERT_TRUE(Trylerp(res, from, to, 0.5));
  EXPECT_ROW1_EQ(1.0f, 0.5f, 0.0f, 0.0f, res);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 0.0f, res);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, res);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);

  to.SetIdentity();
  to.Skew(45.0, 0.0);
  ASSERT_TRUE(Trylerp(res, from, to, 0.25));
  EXPECT_ROW1_EQ(1.0f, 0.25f, 0.0f, 0.0f, res);
  EXPECT_ROW2_EQ(0.0f, 1.0f,  0.0f, 0.0f, res);
  EXPECT_ROW3_EQ(0.0f, 0.0f,  1.0f, 0.0f, res);
  EXPECT_ROW4_EQ(0.0f, 0.0f,  0.0f, 1.0f, res);

  to.SetIdentity();
  to.Skew(45.0, 0.0);
  ASSERT_TRUE(Trylerp(res, from, to, 1.0));
  EXPECT_ROW1_EQ(1.0f, 1.0f, 0.0f, 0.0f, res);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 0.0f, res);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, res);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);

  // NOTE CAREFULLY: Decomposition of skew and rotation terms of the matrix
  // is inherently underconstrained, and so it does not always compute the
  // originally intended skew parameters. The current implementation uses QR
  // decomposition, which decomposes the shear into a rotation + non-uniform
  // scale.
  //
  // It is unlikely that the decomposition implementation will need to change
  // very often, so to get any test coverage, the compromise is to verify the
  // exact matrix that the.Blend() operation produces.
  //
  // This problem also potentially exists for skew along the X axis, but the
  // current QR decomposition implementation just happens to decompose those
  // test matrices intuitively.
  //
  // Unfortunately, this case suffers from uncomfortably large precision
  // error.

  from.SetIdentity();
  from.Skew(0.0, 0.0);

  to.SetIdentity();

  to.Skew(0.0, 45.0);
  ASSERT_TRUE(Trylerp(res, from, to, 0.0));
  EXPECT_EQ(from, res);

  to.SetIdentity();
  to.Skew(0.0, 45.0);
  ASSERT_TRUE(Trylerp(res, from, to, 0.25));
  EXPECT_ROW1_NEAR(
      1.0823489449280947471976333,
      0.0464370719145053845178239,
      0.0,
      0.0,
      res,
      LOOSE_ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(
      0.2152925909665224513123150,
      0.9541702441750861130032035,
      0.0,
      0.0,
      res,
      LOOSE_ERROR_THRESHOLD);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, res);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);

  to.SetIdentity();
  to.Skew(0.0, 45.0);
  ASSERT_TRUE(Trylerp(res, from, to, 0.5));
  EXPECT_ROW1_NEAR(
      1.1152212925809066312865525,
      0.0676495144007326631996335,
      0.0,
      0.0,
      res,
      LOOSE_ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(
      0.4619397844342648662419037,
      0.9519009045724774464858342,
      0.0,
      0.0,
      res,
      LOOSE_ERROR_THRESHOLD);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, res);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);

  to.SetIdentity();
  to.Skew(0.0, 45.0);
  ASSERT_TRUE(Trylerp(res, from, to, 1.0));
  EXPECT_ROW1_NEAR(1.0, 0.0, 0.0, 0.0, res, LOOSE_ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(1.0, 1.0, 0.0, 0.0, res, LOOSE_ERROR_THRESHOLD);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, res);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);
}

TEST(Xform3Test, VerifyBlendForRotationAboutX) {
  // Even though.Blending uses quaternions, axis-aligned rotations should.
  // Blend the same with quaternions or Euler angles. So we can test
  // rotation.Blending by comparing against manually specified matrices from
  // Euler angles.

  Xform3 from = Xform3::Identity();
  from.RotateAboutUnit(Vector3(1.0, 0.0, 0.0), 0.0);

  Xform3 to = Xform3::Identity();

  to.RotateAboutUnit(Vector3(1.0, 0.0, 0.0), Angle::DegreesToRadians(90.0));

  Xform3 res(Xform3::SkipInit);
  ASSERT_TRUE(Trylerp(res, from, to, 0.0));
  EXPECT_EQ(from, res);

  double expected_rotation_angle = Angle::DegreesToRadians(22.5);
  to.SetIdentity();
  to.RotateAboutUnit(Vector3(1.0, 0.0, 0.0), Angle::DegreesToRadians(90.0));
  ASSERT_TRUE(Trylerp(res, from, to, 0.25));
  EXPECT_ROW1_NEAR(1.0, 0.0, 0.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(
      0.0,
      Cos(expected_rotation_angle),
      -Sin(expected_rotation_angle),
      0.0,
      res, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(
      0.0,
      Sin(expected_rotation_angle),
      Cos(expected_rotation_angle),
      0.0,
      res, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);

  expected_rotation_angle = Angle::DegreesToRadians(45.0);
  to.SetIdentity();
  to.RotateAboutUnit(Vector3(1.0, 0.0, 0.0), Angle::DegreesToRadians(90.0));
  ASSERT_TRUE(Trylerp(res, from, to, 0.5));
  EXPECT_ROW1_NEAR(1.0, 0.0, 0.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(
      0.0,
      Cos(expected_rotation_angle),
      -Sin(expected_rotation_angle),
      0.0,
      res, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(
      0.0,
      Sin(expected_rotation_angle),
      Cos(expected_rotation_angle),
      0.0,
      res, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);

  to.SetIdentity();
  to.RotateAboutUnit(Vector3(1.0, 0.0, 0.0), Angle::DegreesToRadians(90.0));
  ASSERT_TRUE(Trylerp(res, from, to, 1.0));
  EXPECT_ROW1_NEAR(1.0, 0.0,  0.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(0.0, 0.0, -1.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(0.0, 1.0,  0.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);
}

TEST(Xform3Test, VerifyBlendForRotationAboutY) {
  Xform3 from = Xform3::Identity();
  from.RotateAbout(Vector3(0.0, 1.0, 0.0), 0.0);

  Xform3 to = Xform3::Identity();

  to.RotateAboutUnit(Vector3(0.0, 1.0, 0.0), Angle::DegreesToRadians(90.0));

  Xform3 res(Xform3::SkipInit);
  ASSERT_TRUE(Trylerp(res, from, to, 0.0));
  EXPECT_EQ(from, res);

  double expected_rotation_angle = Angle::DegreesToRadians(22.5);
  to.SetIdentity();
  to.RotateAboutUnit(Vector3(0.0, 1.0, 0.0), Angle::DegreesToRadians(90.0));
  ASSERT_TRUE(Trylerp(res, from, to, 0.25));
  EXPECT_ROW1_NEAR(
      Cos(expected_rotation_angle),
      0.0,
      Sin(expected_rotation_angle),
      0.0,
      res,
      ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(0.0, 1.0, 0.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(
      -Sin(expected_rotation_angle),
      0.0,
      Cos(expected_rotation_angle),
      0.0,
      res,
      ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);

  expected_rotation_angle = Angle::DegreesToRadians(45.0);
  to.SetIdentity();
  to.RotateAboutUnit(Vector3(0.0, 1.0, 0.0), Angle::DegreesToRadians(90.0));
  ASSERT_TRUE(Trylerp(res, from, to, 0.5));

  EXPECT_ROW1_NEAR(
      Cos(expected_rotation_angle),
      0.0,
      Sin(expected_rotation_angle),
      0.0,
      res,
      ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(0.0, 1.0, 0.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(
      -Sin(expected_rotation_angle),
      0.0,
      Cos(expected_rotation_angle),
      0.0,
      res,
      ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);

  to.SetIdentity();
  to.RotateAboutUnit(Vector3(0.0, 1.0, 0.0), Angle::DegreesToRadians(90.0));
  ASSERT_TRUE(Trylerp(res, from, to, 1.0));
  EXPECT_ROW1_NEAR(0.0,  0.0, 1.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(0.0,  1.0, 0.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(-1.0, 0.0, 0.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);
}

TEST(Xform3Test, VerifyBlendForRotationAboutZ) {
  Xform3 from = Xform3::Identity();
  from.RotateAboutUnit(Vector3(0.0, 0.0, 1.0), 0.0);

  Xform3 to = Xform3::Identity();

  to.RotateAboutUnit(Vector3(0.0, 0.0, 1.0), Angle::DegreesToRadians(90.0));

  Xform3 res(Xform3::SkipInit);
  ASSERT_TRUE(Trylerp(res, from, to, 0.0));
  EXPECT_EQ(from, res);

  double expected_rotation_angle = Angle::DegreesToRadians(22.5);
  to.SetIdentity();
  to.RotateAboutUnit(Vector3(0.0, 0.0, 1.0), Angle::DegreesToRadians(90.0));
  ASSERT_TRUE(Trylerp(res, from, to, 0.25));
  EXPECT_ROW1_NEAR(
      Cos(expected_rotation_angle),
      -Sin(expected_rotation_angle),
      0.0,
      0.0,
      res,
      ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(
      Sin(expected_rotation_angle),
      Cos(expected_rotation_angle),
      0.0,
      0.0,
      res,
      ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(0.0, 0.0, 1.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);

  expected_rotation_angle = Angle::DegreesToRadians(45.0);
  to.SetIdentity();
  to.RotateAboutUnit(Vector3(0.0, 0.0, 1.0), Angle::DegreesToRadians(90.0));
  ASSERT_TRUE(Trylerp(res, from, to, 0.5));
  EXPECT_ROW1_NEAR(
      Cos(expected_rotation_angle),
      -Sin(expected_rotation_angle),
      0.0,
      0.0,
      res,
      ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(
      Sin(expected_rotation_angle),
      Cos(expected_rotation_angle),
      0.0,
      0.0,
      res,
      ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(0.0, 0.0, 1.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);

  to.SetIdentity();
  to.RotateAboutUnit(Vector3(0.0, 0.0, 1.0), Angle::DegreesToRadians(90.0));
  ASSERT_TRUE(Trylerp(res, from, to, 1.0));
  EXPECT_ROW1_NEAR(0.0, -1.0, 0.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(1.0,  0.0, 0.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(0.0,  0.0, 1.0, 0.0, res, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, res);
}

TEST(Xform3Test, VerifyBlendForCompositeTransform) {
  // Verify that the.Blending was done with a decomposition in correct order
  // by blending a composite transform. Using matrix x vector notation
  // (Ax = b, where x is column vector), the ordering should be:
  // perspective * translation * rotation * skew * scale
  //
  // It is not as important (or meaningful) to check intermediate
  // interpolations; order of operations will be tested well enough by the
  // end cases that are easier to specify.

  Xform3 from = Xform3::Identity();
  Xform3 to = Xform3::Identity();

  Xform3 expected_end_of_animation = Xform3::Identity();
  expected_end_of_animation.ApplyPerspectiveDepth(1.0);
  expected_end_of_animation.Translate(10.0, 20.0, 30.0);
  expected_end_of_animation.RotateAboutUnit(Vector3(0.0, 0.0, 1.0), Angle::DegreesToRadians(25.0));
  expected_end_of_animation.Skew(0.0, 45.0);
  expected_end_of_animation.Scale(6.0, 7.0, 8.0);

  to = expected_end_of_animation;

  Xform3 res(Xform3::SkipInit);
  ASSERT_TRUE(Trylerp(res, from, to, 0.0));
  EXPECT_EQ(from, res);

  to = expected_end_of_animation;
  // We short circuit if blend is >= 1, so to check the numerics, we will
  // check that we get close to what we expect when we're nearly done
  // interpolating.
  ASSERT_TRUE(Trylerp(res, from, to, 0.9999f));

  // Recomposing the matrix results in a normalized matrix, so to verify we
  // need to normalize the expected_end_of_animation before comparing elements.
  // Normalizing means dividing everything by expected_end_of_animation.m44().
  Xform3 normalized_expected_end_of_animation = expected_end_of_animation;
  Xform3 normalization_matrix = Xform3::Identity();
  normalization_matrix.set(0, 0, 1 / expected_end_of_animation.get(3, 3));
  normalization_matrix.set(1, 1, 1 / expected_end_of_animation.get(3, 3));
  normalization_matrix.set(2, 2, 1 / expected_end_of_animation.get(3, 3));
  normalization_matrix.set(3, 3, 1 / expected_end_of_animation.get(3, 3));
  normalized_expected_end_of_animation.Concat(normalization_matrix);

  EXPECT_TRUE(MatricesAreNearlyEqual(normalized_expected_end_of_animation, res));
}

TEST(Xform3Test, DecomposedXformCtor) {
  DecomposedXform3 decomp(DecomposedXform3::InitWithIdentity);
  EXPECT_TRUE(decomp.translate.IsZero());
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(1.0, decomp.scale[i]);
    EXPECT_EQ(0.0, decomp.shear[i]);
    EXPECT_EQ(0.0, decomp.perspective[i]);
  }
  EXPECT_TRUE(decomp.quaternion.IsIdentity());
  EXPECT_EQ(1.0, decomp.perspective[3]);
  Xform3 identity = Xform3::Identity();
  Xform3 composed = Xform3::FromDecomposed(decomp);
  EXPECT_TRUE(MatricesAreNearlyEqual(identity, composed));
}

TEST(Xform3Test, FactorTRS) {
  for (int degrees = 0; degrees < 180; ++degrees) {
    // build a transformation matrix.
    Xform3 transform = Xform3::Identity();
    transform.Translate2D(degrees * 2, -degrees * 3);
    transform.Rotate2D(Angle::DegreesToRadians(degrees));
    transform.Scale2D(degrees + 1, 2 * degrees + 1);

    // factor the matrix
    DecomposedXform3 decomp(DecomposedXform3::SkipInit);
    bool success = transform.Decompose(decomp);
    EXPECT_TRUE(success);
    EXPECT_FLOAT_EQ(decomp.translate.x, degrees * 2);
    EXPECT_FLOAT_EQ(decomp.translate.y, -degrees * 3);
    double rotation = Angle::RadiansToDegrees(2 * Acos(decomp.quaternion.w));
    while (rotation < 0.0)
      rotation += 360.0;
    while (rotation > 360.0)
      rotation -= 360.0;

    const float epsilon = 0.00015f;
    EXPECT_NEAR(rotation, degrees, epsilon);
    EXPECT_NEAR(decomp.scale[0], degrees + 1, epsilon);
    EXPECT_NEAR(decomp.scale[1], 2 * degrees + 1, epsilon);
  }
}

TEST(Xform3Test, DecomposeTransform) {
  for (float scale = 0.001f; scale < 2.0f; scale += 0.001f) {
    Xform3 transform = Xform3::Identity();
    transform.Scale2D(scale, scale);
    EXPECT_TRUE(transform.Preserves2DAxisAlignment());

    DecomposedXform3 decomp(DecomposedXform3::SkipInit);
    bool success = transform.Decompose(decomp);
    EXPECT_TRUE(success);

    Xform3 compose_transform = Xform3::FromDecomposed(decomp);
    EXPECT_TRUE(compose_transform.Preserves2DAxisAlignment());
  }
}

TEST(Xform3Test, IntegerTranslation) {
  Xform3 transform = Xform3::Identity();
  EXPECT_TRUE(transform.IsIntegerTranslate());

  transform.Translate(1, 2, 3);
  EXPECT_TRUE(transform.IsIntegerTranslate());

  transform.SetIdentity();
  transform.Translate(-1, -2, -3);
  EXPECT_TRUE(transform.IsIntegerTranslate());

  transform.SetIdentity();
  transform.Translate(4.5f, 0, 0);
  EXPECT_FALSE(transform.IsIntegerTranslate());

  transform.SetIdentity();
  transform.Translate(0, -6.7f, 0);
  EXPECT_FALSE(transform.IsIntegerTranslate());

  transform.SetIdentity();
  transform.Translate(0, 0, 8.9f);
  EXPECT_FALSE(transform.IsIntegerTranslate());
}

TEST(Xform3Test, VerifyMatrixInversion) {
  {
    // Invert a translation
    Xform3 translation = Xform3::Identity();
    translation.Translate(2.0, 3.0, 4.0);
    EXPECT_TRUE(translation.IsInvertible());

    Xform3 inverted(Xform3::SkipInit);
    ASSERT_TRUE(translation.GetInverted(inverted));
    EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, -2.0f, inverted);
    EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, -3.0f, inverted);
    EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, -4.0f, inverted);
    EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f,  1.0f, inverted);
  }

  {
    // Invert a non-uniform scale
    Xform3 scale = Xform3::Identity();
    scale.Scale(4.0, 10.0, 100.0);
    EXPECT_TRUE(scale.IsInvertible());

    Xform3 inverted(Xform3::SkipInit);
    ASSERT_TRUE(scale.GetInverted(inverted));
    EXPECT_ROW1_EQ(0.25f, 0.0f, 0.0f, 0.0f, inverted);
    EXPECT_ROW2_EQ(0.0f,  0.1f, 0.0f, 0.0f, inverted);
    EXPECT_ROW3_EQ(0.0f,  0.0f, 0.01f, 0.0f, inverted);
    EXPECT_ROW4_EQ(0.0f,  0.0f, 0.0f, 1.0f, inverted);
  }

  {
    // Try to invert a matrix that is not invertible.
    // The inverse() function should reset the output matrix to identity.
    Xform3 uninvertible = Xform3::Identity();
    uninvertible.set(0, 0, 0.f);
    uninvertible.set(1, 1, 0.f);
    uninvertible.set(2, 2, 0.f);
    uninvertible.set(3, 3, 0.f);
    EXPECT_FALSE(uninvertible.IsInvertible());
  }
}

TEST(Xform3Test, VerifyBackfaceVisibilityBasicCases) {
  Xform3 transform = Xform3::Identity();

  transform.SetIdentity();
  EXPECT_FALSE(transform.IsBackFaceVisible());

  transform.SetIdentity();
  transform.RotateAboutYAxis(Angle::DegreesToRadians(80.0));
  EXPECT_FALSE(transform.IsBackFaceVisible());

  transform.SetIdentity();
  transform.RotateAboutYAxis(Angle::DegreesToRadians(100.0));
  EXPECT_TRUE(transform.IsBackFaceVisible());

  // Edge case, 90 degree rotation should return false.
  transform.SetIdentity();
  transform.RotateAboutYAxis(Angle::DegreesToRadians(90.0));
  EXPECT_FALSE(transform.IsBackFaceVisible());
}

TEST(Xform3Test, VerifyBackfaceVisibilityForPerspective) {
  Xform3 layer_space_to_projection_plane = Xform3::Identity();

  // This tests if IsBackFaceVisible works properly under perspective
  // transforms.  Specifically, layers that may have their back face visible in
  // orthographic projection, may not actually have back face visible under
  // perspective projection.

  // Case 1: Layer is rotated by slightly more than 90 degrees, at the center
  //         of the prespective projection. In this case, the layer's back-side
  //         is visible to the camera.
  layer_space_to_projection_plane.SetIdentity();
  layer_space_to_projection_plane.ApplyPerspectiveDepth(1.0);
  layer_space_to_projection_plane.Translate(0.0, 0.0, 0.0);
  layer_space_to_projection_plane.RotateAboutYAxis(Angle::DegreesToRadians(100.0));
  EXPECT_TRUE(layer_space_to_projection_plane.IsBackFaceVisible());

  // Case 2: Layer is rotated by slightly more than 90 degrees, but shifted off
  //         to the side of the camera. Because of the wide field-of-view, the
  //         layer's front side is still visible.
  //
  //                       |<-- front side of layer is visible to camera
  //                    \  |            /
  //                     \ |           /
  //                      \|          /
  //                       |         /
  //                       |\       /<-- camera field of view
  //                       | \     /
  // back side of layer -->|  \   /
  //                           \./ <-- camera origin
  //
  layer_space_to_projection_plane.SetIdentity();
  layer_space_to_projection_plane.ApplyPerspectiveDepth(1.0);
  layer_space_to_projection_plane.Translate(-10.0, 0.0, 0.0);
  layer_space_to_projection_plane.RotateAboutYAxis(Angle::DegreesToRadians(100.0));
  EXPECT_FALSE(layer_space_to_projection_plane.IsBackFaceVisible());

  // Case 3: Additionally rotating the layer by 180 degrees should of course
  //         show the opposite result of case 2.
  layer_space_to_projection_plane.RotateAboutYAxis(Angle::DegreesToRadians(180.0));
  EXPECT_TRUE(layer_space_to_projection_plane.IsBackFaceVisible());
}

TEST(Xform3Test, VerifyDefaultConstructorCreatesIdentityMatrix) {
  Xform3 A = Xform3::Identity();
  EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, 0.0f, A);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 0.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);
  EXPECT_TRUE(A.IsIdentity());
}

TEST(Xform3Test, VerifyCopyConstructor) {
  Xform3 A = Xform3::Identity();
  InitializeTestMatrix(A);

  // Copy constructor should produce exact same elements as matrix A.
  Xform3 B(A);
  EXPECT_ROW1_EQ(10.0f, 14.0f, 18.0f, 22.0f, B);
  EXPECT_ROW2_EQ(11.0f, 15.0f, 19.0f, 23.0f, B);
  EXPECT_ROW3_EQ(12.0f, 16.0f, 20.0f, 24.0f, B);
  EXPECT_ROW4_EQ(13.0f, 17.0f, 21.0f, 25.0f, B);
}

TEST(Xform3Test, VerifyConstructorFor16Elements) {
  Xform3 transform(
      1.0, 2.0, 3.0, 4.0,
      5.0, 6.0, 7.0, 8.0,
      9.0, 10.0, 11.0, 12.0,
      13.0, 14.0, 15.0, 16.0);

  EXPECT_ROW1_EQ(1.0f, 2.0f, 3.0f, 4.0f, transform);
  EXPECT_ROW2_EQ(5.0f, 6.0f, 7.0f, 8.0f, transform);
  EXPECT_ROW3_EQ(9.0f, 10.0f, 11.0f, 12.0f, transform);
  EXPECT_ROW4_EQ(13.0f, 14.0f, 15.0f, 16.0f, transform);
}

TEST(Xform3Test, VerifyConstructorFor2dElements) {
  Xform3 transform(
      1, 2,
      3, 4,
      5, 6);

  EXPECT_ROW1_EQ(1, 3, 0, 5, transform);
  EXPECT_ROW2_EQ(2, 4, 0, 6, transform);
  EXPECT_ROW3_EQ(0, 0, 1, 0, transform);
  EXPECT_ROW4_EQ(0, 0, 0, 1, transform);
}

TEST(Xform3Test, VerifyAssignmentOperator) {
  Xform3 A = Xform3::Identity();
  InitializeTestMatrix(A);
  Xform3 B = Xform3::Identity();
  InitializeTestMatrix2(B);
  Xform3 C = Xform3::Identity();
  InitializeTestMatrix2(C);
  C = B = A;

  // Both B and C should now have been re-assigned to the value of A.
  EXPECT_ROW1_EQ(10.0f, 14.0f, 18.0f, 22.0f, B);
  EXPECT_ROW2_EQ(11.0f, 15.0f, 19.0f, 23.0f, B);
  EXPECT_ROW3_EQ(12.0f, 16.0f, 20.0f, 24.0f, B);
  EXPECT_ROW4_EQ(13.0f, 17.0f, 21.0f, 25.0f, B);

  EXPECT_ROW1_EQ(10.0f, 14.0f, 18.0f, 22.0f, C);
  EXPECT_ROW2_EQ(11.0f, 15.0f, 19.0f, 23.0f, C);
  EXPECT_ROW3_EQ(12.0f, 16.0f, 20.0f, 24.0f, C);
  EXPECT_ROW4_EQ(13.0f, 17.0f, 21.0f, 25.0f, C);
}

TEST(Xform3Test, VerifyEqualsBooleanOperator) {
  Xform3 A = Xform3::Identity();
  InitializeTestMatrix(A);

  Xform3 B = Xform3::Identity();
  InitializeTestMatrix(B);
  EXPECT_TRUE(A == B);

  // Modifying multiple elements should cause equals operator to return false.
  Xform3 C = Xform3::Identity();
  InitializeTestMatrix2(C);
  EXPECT_FALSE(A == C);

  // Modifying any one individual element should cause equals operator to
  // return false.
  Xform3 D = Xform3::Identity();
  D = A;
  D.set(0, 0, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(1, 0, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(2, 0, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(3, 0, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(0, 1, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(1, 1, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(2, 1, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(3, 1, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(0, 2, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(1, 2, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(2, 2, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(3, 2, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(0, 3, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(1, 3, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(2, 3, 0.f);
  EXPECT_FALSE(A == D);

  D = A;
  D.set(3, 3, 0.f);
  EXPECT_FALSE(A == D);
}

TEST(Xform3Test, VerifyMultiplyOperator) {
  Xform3 A = Xform3::Identity();
  InitializeTestMatrix(A);

  Xform3 B = Xform3::Identity();
  InitializeTestMatrix2(B);

  Xform3 C = A * B;
  EXPECT_ROW1_EQ(2036.0f, 2292.0f, 2548.0f, 2804.0f, C);
  EXPECT_ROW2_EQ(2162.0f, 2434.0f, 2706.0f, 2978.0f, C);
  EXPECT_ROW3_EQ(2288.0f, 2576.0f, 2864.0f, 3152.0f, C);
  EXPECT_ROW4_EQ(2414.0f, 2718.0f, 3022.0f, 3326.0f, C);

  // Just an additional sanity check; matrix multiplication is not commutative.
  EXPECT_FALSE(A * B == B * A);
}

TEST(Xform3Test, VerifyMultiplyAndAssignOperator) {
  Xform3 A = Xform3::Identity();
  InitializeTestMatrix(A);

  Xform3 B = Xform3::Identity();
  InitializeTestMatrix2(B);

  A *= B;
  EXPECT_ROW1_EQ(2036.0f, 2292.0f, 2548.0f, 2804.0f, A);
  EXPECT_ROW2_EQ(2162.0f, 2434.0f, 2706.0f, 2978.0f, A);
  EXPECT_ROW3_EQ(2288.0f, 2576.0f, 2864.0f, 3152.0f, A);
  EXPECT_ROW4_EQ(2414.0f, 2718.0f, 3022.0f, 3326.0f, A);

  // Just an additional sanity check; matrix multiplication is not commutative.
  Xform3 C = A;
  C *= B;
  Xform3 D = B;
  D *= A;
  EXPECT_FALSE(C == D);
}

TEST(Xform3Test, VerifyMatrixMultiplication) {
  Xform3 A = Xform3::Identity();
  InitializeTestMatrix(A);

  Xform3 B = Xform3::Identity();
  InitializeTestMatrix2(B);

  A.Concat(B);
  EXPECT_ROW1_EQ(2036.0f, 2292.0f, 2548.0f, 2804.0f, A);
  EXPECT_ROW2_EQ(2162.0f, 2434.0f, 2706.0f, 2978.0f, A);
  EXPECT_ROW3_EQ(2288.0f, 2576.0f, 2864.0f, 3152.0f, A);
  EXPECT_ROW4_EQ(2414.0f, 2718.0f, 3022.0f, 3326.0f, A);
}

TEST(Xform3Test, VerifyMakeIdentiy) {
  Xform3 A = Xform3::Identity();
  InitializeTestMatrix(A);
  A.SetIdentity();
  EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, 0.0f, A);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 0.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);
  EXPECT_TRUE(A.IsIdentity());
}

TEST(Xform3Test, VerifyTranslate) {
  Xform3 A = Xform3::Identity();
  A.Translate2D(2.0, 3.0);
  EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, 2.0f, A);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 3.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Verify that Translate() post-multiplies the existing matrix.
  A.SetIdentity();
  A.Scale2D(5.0, 5.0);
  A.Translate2D(2.0, 3.0);
  EXPECT_ROW1_EQ(5.0f, 0.0f, 0.0f, 10.0f, A);
  EXPECT_ROW2_EQ(0.0f, 5.0f, 0.0f, 15.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f,  A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f,  A);
}

TEST(Xform3Test, VerifyTranslate3d) {
  Xform3 A = Xform3::Identity();
  A.Translate(2.0, 3.0, 4.0);
  EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, 2.0f, A);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 3.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 4.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Verify that Translate() post-multiplies the existing matrix.
  A.SetIdentity();
  A.Scale(6.0, 7.0, 8.0);
  A.Translate(2.0, 3.0, 4.0);
  EXPECT_ROW1_EQ(6.0f, 0.0f, 0.0f, 12.0f, A);
  EXPECT_ROW2_EQ(0.0f, 7.0f, 0.0f, 21.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 8.0f, 32.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f,  A);
}

TEST(Xform3Test, VerifyScale) {
  Xform3 A = Xform3::Identity();
  A.Scale2D(6.0, 7.0);
  EXPECT_ROW1_EQ(6.0f, 0.0f, 0.0f, 0.0f, A);
  EXPECT_ROW2_EQ(0.0f, 7.0f, 0.0f, 0.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Verify that Scale() post-multiplies the existing matrix.
  A.SetIdentity();
  A.Translate(2.0, 3.0, 4.0);
  A.Scale2D(6.0, 7.0);
  EXPECT_ROW1_EQ(6.0f, 0.0f, 0.0f, 2.0f, A);
  EXPECT_ROW2_EQ(0.0f, 7.0f, 0.0f, 3.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 4.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);
}

TEST(Xform3Test, VerifyScale3d) {
  Xform3 A = Xform3::Identity();
  A.Scale(6.0, 7.0, 8.0);
  EXPECT_ROW1_EQ(6.0f, 0.0f, 0.0f, 0.0f, A);
  EXPECT_ROW2_EQ(0.0f, 7.0f, 0.0f, 0.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 8.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Verify that scale3d() post-multiplies the existing matrix.
  A.SetIdentity();
  A.Translate(2.0, 3.0, 4.0);
  A.Scale(6.0, 7.0, 8.0);
  EXPECT_ROW1_EQ(6.0f, 0.0f, 0.0f, 2.0f, A);
  EXPECT_ROW2_EQ(0.0f, 7.0f, 0.0f, 3.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 8.0f, 4.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);
}

TEST(Xform3Test, VerifyRotate) {
  Xform3 A = Xform3::Identity();
  A.Rotate2D(Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_NEAR(0.0, -1.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(1.0, 0.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Verify that Rotate() post-multiplies the existing matrix.
  A.SetIdentity();
  A.Scale(6.0, 7.0, 8.0);
  A.Rotate2D(Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_NEAR(0.0, -6.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(7.0, 0.0,  0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 8.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);
}

TEST(Xform3Test, VerifyRotateAboutXAxis) {
  Xform3 A = Xform3::Identity();
  double sin45 = 0.5 * sqrt(2.0);
  double cos45 = sin45;

  A.SetIdentity();
  A.RotateAboutXAxis(Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, 0.0f, A);
  EXPECT_ROW2_NEAR(0.0, 0.0, -1.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(0.0, 1.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  A.SetIdentity();
  A.RotateAboutXAxis(Angle::DegreesToRadians(45.0));
  EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, 0.0f, A);
  EXPECT_ROW2_NEAR(0.0, cos45, -sin45, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(0.0, sin45, cos45, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Verify that RotateAboutXAxis(angle) post-multiplies the existing matrix.
  A.SetIdentity();
  A.Scale(6.0, 7.0, 8.0);
  A.RotateAboutXAxis(Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_NEAR(6.0, 0.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(0.0, 0.0, -7.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(0.0, 8.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);
}

TEST(Xform3Test, VerifyRotateAboutYAxis) {
  Xform3 A = Xform3::Identity();
  double sin45 = 0.5 * sqrt(2.0);
  double cos45 = sin45;

  // Note carefully, the expected pattern is inverted compared to rotating
  // about x axis or z axis.
  A.SetIdentity();
  A.RotateAboutYAxis(Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_NEAR(0.0, 0.0, 1.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 0.0f, A);
  EXPECT_ROW3_NEAR(-1.0, 0.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  A.SetIdentity();
  A.RotateAboutYAxis(Angle::DegreesToRadians(45.0));
  EXPECT_ROW1_NEAR(cos45, 0.0, sin45, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 0.0f, A);
  EXPECT_ROW3_NEAR(-sin45, 0.0, cos45, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Verify that RotateAboutYAxis(angle) post-multiplies the existing matrix.
  A.SetIdentity();
  A.Scale(6.0, 7.0, 8.0);
  A.RotateAboutYAxis(Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_NEAR(0.0, 0.0, 6.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(0.0, 7.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(-8.0, 0.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);
}

TEST(Xform3Test, VerifyRotateAboutZAxis) {
  Xform3 A = Xform3::Identity();
  double sin45 = 0.5 * sqrt(2.0);
  double cos45 = sin45;

  A.SetIdentity();
  A.RotateAboutZAxis(Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_NEAR(0.0, -1.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(1.0, 0.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  A.SetIdentity();
  A.RotateAboutZAxis(Angle::DegreesToRadians(45.0));
  EXPECT_ROW1_NEAR(cos45, -sin45, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(sin45, cos45, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Verify that RotateAboutZAxis(angle) post-multiplies the existing matrix.
  A.SetIdentity();
  A.Scale(6.0, 7.0, 8.0);
  A.RotateAboutZAxis(Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_NEAR(0.0, -6.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(7.0, 0.0,  0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 8.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);
}

TEST(Xform3Test, VerifyRotateAboutForAlignedAxes) {
  Xform3 A = Xform3::Identity();

  // Check rotation about z-axis
  A.SetIdentity();
  A.RotateAbout(Vector3(0.0, 0.0, 1.0), Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_NEAR(0.0, -1.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(1.0, 0.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Check rotation about x-axis
  A.SetIdentity();
  A.RotateAbout(Vector3(1.0, 0.0, 0.0), Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, 0.0f, A);
  EXPECT_ROW2_NEAR(0.0, 0.0, -1.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(0.0, 1.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Check rotation about y-axis. Note carefully, the expected pattern is
  // inverted compared to rotating about x axis or z axis.
  A.SetIdentity();
  A.RotateAbout(Vector3(0.0, 1.0, 0.0), Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_NEAR(0.0, 0.0, 1.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 0.0f, A);
  EXPECT_ROW3_NEAR(-1.0, 0.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Verify that rotate3d(axis, angle) post-multiplies the existing matrix.
  A.SetIdentity();
  A.Scale(6.0, 7.0, 8.0);
  A.RotateAboutZAxis(Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_NEAR(0.0, -6.0, 0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(7.0, 0.0,  0.0, 0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 8.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);
}

TEST(Xform3Test, VerifyRotateAboutForArbitraryAxis) {
  // Check rotation about an arbitrary non-axis-aligned vector.
  Xform3 A = Xform3::Identity();
  A.RotateAbout(Vector3(1.0, 1.0, 1.0), Angle::DegreesToRadians(90.0));
  EXPECT_ROW1_NEAR(
      0.3333333333333334258519187,
      -0.2440169358562924717404030,
      0.9106836025229592124219380,
      0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW2_NEAR(
      0.9106836025229592124219380,
      0.3333333333333334258519187,
      -0.2440169358562924717404030,
      0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW3_NEAR(
      -0.2440169358562924717404030,
      0.9106836025229592124219380,
      0.3333333333333334258519187,
      0.0, A, ERROR_THRESHOLD);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);
}

TEST(Xform3Test, VerifyRotateAboutForDegenerateAxis) {
  // Check rotation about a degenerate zero vector.
  // It is expected to skip applying the rotation.
  Xform3 A = Xform3::Identity();

  A.RotateAbout(Vector3(0.0, 0.0, 0.0), Angle::DegreesToRadians(45.0));
  // Verify that A remains unchanged.
  EXPECT_TRUE(A.IsIdentity());

  InitializeTestMatrix(A);
  A.RotateAbout(Vector3(0.0, 0.0, 0.0), Angle::DegreesToRadians(35.0));

  // Verify that A remains unchanged.
  EXPECT_ROW1_EQ(10.0f, 14.0f, 18.0f, 22.0f, A);
  EXPECT_ROW2_EQ(11.0f, 15.0f, 19.0f, 23.0f, A);
  EXPECT_ROW3_EQ(12.0f, 16.0f, 20.0f, 24.0f, A);
  EXPECT_ROW4_EQ(13.0f, 17.0f, 21.0f, 25.0f, A);
}

TEST(Xform3Test, VerifySkew) {
  // Test a skew along X axis only
  Xform3 A = Xform3::Identity();
  A.Skew(45.0, 0.0);
  EXPECT_ROW1_EQ(1.0f, 1.0f, 0.0f, 0.0f, A);
  EXPECT_ROW2_EQ(0.0f, 1.0f, 0.0f, 0.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Test a skew along Y axis only
  A.SetIdentity();
  A.Skew(0.0, 45.0);
  EXPECT_ROW1_EQ(1.0f, 0.0f, 0.0f, 0.0f, A);
  EXPECT_ROW2_EQ(1.0f, 1.0f, 0.0f, 0.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Verify that skew() post-multiplies the existing matrix. Row 1, column 2,
  // would incorrectly have value "7" if the matrix is pre-multiplied instead
  // of post-multiplied.
  A.SetIdentity();
  A.Scale(6.0, 7.0, 8.0);
  A.Skew(45.0, 0.0);
  EXPECT_ROW1_EQ(6.0f, 6.0f, 0.0f, 0.0f, A);
  EXPECT_ROW2_EQ(0.0f, 7.0f, 0.0f, 0.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 8.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);

  // Test a skew along X and Y axes both
  A.SetIdentity();
  A.Skew(45.0, 45.0);
  EXPECT_ROW1_EQ(1.0f, 1.0f, 0.0f, 0.0f, A);
  EXPECT_ROW2_EQ(1.0f, 1.0f, 0.0f, 0.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, 1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, 0.0f, 1.0f, A);
}

TEST(Xform3Test, VerifyPerspectiveDepth) {
  Xform3 A = Xform3::Identity();
  A.ApplyPerspectiveDepth(1.0);
  EXPECT_ROW1_EQ(1.0f, 0.0f,  0.0f, 0.0f, A);
  EXPECT_ROW2_EQ(0.0f, 1.0f,  0.0f, 0.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f,  1.0f, 0.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, -1.0f, 1.0f, A);

  // Verify that PerspectiveDepth() post-multiplies the existing matrix.
  A.SetIdentity();
  A.Translate(2.0, 3.0, 4.0);
  A.ApplyPerspectiveDepth(1.0);
  EXPECT_ROW1_EQ(1.0f, 0.0f, -2.0f, 2.0f, A);
  EXPECT_ROW2_EQ(0.0f, 1.0f, -3.0f, 3.0f, A);
  EXPECT_ROW3_EQ(0.0f, 0.0f, -3.0f, 4.0f, A);
  EXPECT_ROW4_EQ(0.0f, 0.0f, -1.0f, 1.0f, A);
}

TEST(Xform3Test, VerifyHasPerspective) {
  Xform3 A = Xform3::Identity();
  A.ApplyPerspectiveDepth(1.0);
  EXPECT_TRUE(A.HasPerspective());

  A.SetIdentity();
  A.ApplyPerspectiveDepth(0.0);
  EXPECT_FALSE(A.HasPerspective());

  A.SetIdentity();
  A.set(3, 0, -1.f);
  EXPECT_TRUE(A.HasPerspective());

  A.SetIdentity();
  A.set(3, 1, -1.f);
  EXPECT_TRUE(A.HasPerspective());

  A.SetIdentity();
  A.set(3, 2, -0.3f);
  EXPECT_TRUE(A.HasPerspective());

  A.SetIdentity();
  A.set(3, 3, 0.5f);
  EXPECT_TRUE(A.HasPerspective());

  A.SetIdentity();
  A.set(3, 3, 0.f);
  EXPECT_TRUE(A.HasPerspective());
}

TEST(Xform3Test, VerifyIsInvertible) {
  Xform3 A = Xform3::Identity();

  // Translations, rotations, scales, skews and arbitrary combinations of them
  // are invertible.
  A.SetIdentity();
  EXPECT_TRUE(A.IsInvertible());

  A.SetIdentity();
  A.Translate(2.0, 3.0, 4.0);
  EXPECT_TRUE(A.IsInvertible());

  A.SetIdentity();
  A.Scale(6.0, 7.0, 8.0);
  EXPECT_TRUE(A.IsInvertible());

  A.SetIdentity();
  A.RotateAboutXAxis(Angle::DegreesToRadians(10.0));
  A.RotateAboutYAxis(Angle::DegreesToRadians(20.0));
  A.RotateAboutZAxis(Angle::DegreesToRadians(30.0));
  EXPECT_TRUE(A.IsInvertible());

  A.SetIdentity();
  A.Skew(45.0, 0.0);
  EXPECT_TRUE(A.IsInvertible());

  // A perspective matrix (projection plane at z=0) is invertible. The
  // intuitive explanation is that perspective is eqivalent to a skew of the
  // w-axis; skews are invertible.
  A.SetIdentity();
  A.ApplyPerspectiveDepth(1.0);
  EXPECT_TRUE(A.IsInvertible());

  // A "pure" perspective matrix derived by similar triangles, with m44() set
  // to zero (i.e. camera positioned at the origin), is not invertible.
  A.SetIdentity();
  A.ApplyPerspectiveDepth(1.0);
  A.set(3, 3, 0.f);
  EXPECT_FALSE(A.IsInvertible());

  // Adding more to a non-invertible matrix will not make it invertible in the
  // general case.
  A.SetIdentity();
  A.ApplyPerspectiveDepth(1.0);
  A.set(3, 3, 0.f);
  A.Scale(6.0, 7.0, 8.0);
  A.RotateAboutXAxis(Angle::DegreesToRadians(10.0));
  A.RotateAboutYAxis(Angle::DegreesToRadians(20.0));
  A.RotateAboutZAxis(Angle::DegreesToRadians(30.0));
  A.Translate(6.0, 7.0, 8.0);
  EXPECT_FALSE(A.IsInvertible());

  // A degenerate matrix of all zeros is not invertible.
  A.SetIdentity();
  A.set(0, 0, 0.f);
  A.set(1, 1, 0.f);
  A.set(2, 2, 0.f);
  A.set(3, 3, 0.f);
  EXPECT_FALSE(A.IsInvertible());
}

TEST(Xform3Test, VerifyIsIdentity) {
  Xform3 A = Xform3::Identity();

  InitializeTestMatrix(A);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  EXPECT_TRUE(A.IsIdentity());

  // Modifying any one individual element should cause the matrix to no longer
  // be identity.
  A.SetIdentity();
  A.set(0, 0, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(1, 0, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(2, 0, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(3, 0, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(0, 1, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(1, 1, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(2, 1, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(3, 1, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(0, 2, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(1, 2, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(2, 2, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(3, 2, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(0, 3, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(1, 3, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(2, 3, 2.f);
  EXPECT_FALSE(A.IsIdentity());

  A.SetIdentity();
  A.set(3, 3, 2.f);
  EXPECT_FALSE(A.IsIdentity());
}

TEST(Xform3Test, VerifyIsIdentityOrTranslation) {
  Xform3 A = Xform3::Identity();

  InitializeTestMatrix(A);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  EXPECT_TRUE(A.IsTranslate());

  // Modifying any non-translation components should cause
  // IsIdentityOrTranslation() to return false. NOTE: (0, 3), (1, 3), and
  // (2, 3) are the translation components, so modifying them should still
  // return true.
  A.SetIdentity();
  A.set(0, 0, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  A.set(1, 0, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  A.set(2, 0, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  A.set(3, 0, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  A.set(0, 1, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  A.set(1, 1, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  A.set(2, 1, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  A.set(3, 1, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  A.set(0, 2, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  A.set(1, 2, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  A.set(2, 2, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  A.SetIdentity();
  A.set(3, 2, 2.f);
  EXPECT_FALSE(A.IsTranslate());

  // Note carefully - expecting true here.
  A.SetIdentity();
  A.set(0, 3, 2.f);
  EXPECT_TRUE(A.IsTranslate());

  // Note carefully - expecting true here.
  A.SetIdentity();
  A.set(1, 3, 2.f);
  EXPECT_TRUE(A.IsTranslate());

  // Note carefully - expecting true here.
  A.SetIdentity();
  A.set(2, 3, 2.f);
  EXPECT_TRUE(A.IsTranslate());

  A.SetIdentity();
  A.set(3, 3, 2.f);
  EXPECT_FALSE(A.IsTranslate());
}

TEST(Xform3Test, VerifyisNearTranslate) {
  Xform3 A = Xform3::Identity();

  // Exact pure translation.
  A.SetIdentity();

  // Set translate values to values other than 0 or 1.
  A.set(0, 3, 3.4f);
  A.set(1, 3, 4.4f);
  A.set(2, 3, 5.6f);

  EXPECT_TRUE(A.isNearTranslate(0));
  EXPECT_TRUE(A.isNearTranslate(ApproxZero));

  // Approximately pure translation.
  InitializeApproxIdentityMatrix(A);

  // Some values must be exact.
  A.set(3, 0, 0);
  A.set(3, 1, 0);
  A.set(3, 2, 0);
  A.set(3, 3, 1);

  // Set translate values to values other than 0 or 1.
  A.set(0, 3, 3.4f);
  A.set(1, 3, 4.4f);
  A.set(2, 3, 5.6f);

  EXPECT_FALSE(A.isNearTranslate(0));
  EXPECT_TRUE(A.isNearTranslate(ApproxZero));

  // Not approximately pure translation.
  InitializeApproxIdentityMatrix(A);

  // Some values must be exact.
  A.set(3, 0, 0);
  A.set(3, 1, 0);
  A.set(3, 2, 0);
  A.set(3, 3, 1);

  // Set some values (not translate values) to values other than 0 or 1.
  A.set(0, 1, 3.4f);
  A.set(3, 2, 4.4f);
  A.set(2, 0, 5.6f);

  EXPECT_FALSE(A.isNearTranslate(0));
  EXPECT_FALSE(A.isNearTranslate(ApproxZero));
}

TEST(Xform3Test, VerifyIsScaleOrTranslation) {
  Xform3 A = Xform3::Identity();

  InitializeTestMatrix(A);
  EXPECT_FALSE(A.IsScaleTranslate());

  A.SetIdentity();
  EXPECT_TRUE(A.IsScaleTranslate());

  // Modifying any non-scale or non-translation components should cause
  // IsScaleOrTranslation() to return false. (0, 0), (1, 1), (2, 2), (0, 3),
  // (1, 3), and (2, 3) are the scale and translation components, so
  // modifying them should still return true.

  // Note carefully - expecting true here.
  A.SetIdentity();
  A.set(0, 0, 2.f);
  EXPECT_TRUE(A.IsScaleTranslate());

  A.SetIdentity();
  A.set(1, 0, 2.f);
  EXPECT_FALSE(A.IsScaleTranslate());

  A.SetIdentity();
  A.set(2, 0, 2.f);
  EXPECT_FALSE(A.IsScaleTranslate());

  A.SetIdentity();
  A.set(3, 0, 2.f);
  EXPECT_FALSE(A.IsScaleTranslate());

  A.SetIdentity();
  A.set(0, 1, 2.f);
  EXPECT_FALSE(A.IsScaleTranslate());

  // Note carefully - expecting true here.
  A.SetIdentity();
  A.set(1, 1, 2.f);
  EXPECT_TRUE(A.IsScaleTranslate());

  A.SetIdentity();
  A.set(2, 1, 2.f);
  EXPECT_FALSE(A.IsScaleTranslate());

  A.SetIdentity();
  A.set(3, 1, 2.f);
  EXPECT_FALSE(A.IsScaleTranslate());

  A.SetIdentity();
  A.set(0, 2, 2.f);
  EXPECT_FALSE(A.IsScaleTranslate());

  A.SetIdentity();
  A.set(1, 2, 2.f);
  EXPECT_FALSE(A.IsScaleTranslate());

  // Note carefully - expecting true here.
  A.SetIdentity();
  A.set(2, 2, 2.f);
  EXPECT_TRUE(A.IsScaleTranslate());

  A.SetIdentity();
  A.set(3, 2, 2.f);
  EXPECT_FALSE(A.IsScaleTranslate());

  // Note carefully - expecting true here.
  A.SetIdentity();
  A.set(0, 3, 2.f);
  EXPECT_TRUE(A.IsScaleTranslate());

  // Note carefully - expecting true here.
  A.SetIdentity();
  A.set(1, 3, 2.f);
  EXPECT_TRUE(A.IsScaleTranslate());

  // Note carefully - expecting true here.
  A.SetIdentity();
  A.set(2, 3, 2.f);
  EXPECT_TRUE(A.IsScaleTranslate());

  A.SetIdentity();
  A.set(3, 3, 2.f);
  EXPECT_FALSE(A.IsScaleTranslate());
}

TEST(Xform3Test, VerifyFlattenTo2d) {
  Xform3 A = Xform3::Identity();
  InitializeTestMatrix(A);

  A.FlattenTo2D();
  EXPECT_ROW1_EQ(10.0f, 14.0f, 0.0f, 22.0f, A);
  EXPECT_ROW2_EQ(11.0f, 15.0f, 0.0f, 23.0f, A);
  EXPECT_ROW3_EQ(0.0f,  0.0f,  1.0f, 0.0f,  A);
  EXPECT_ROW4_EQ(13.0f, 17.0f, 0.0f, 25.0f, A);
}

TEST(Xform3Test, IsFlat) {
  Xform3 transform = Xform3::Identity();
  InitializeTestMatrix(transform);

  // A transform with all entries non-zero isn't flat.
  EXPECT_FALSE(transform.IsFlat());

  transform.set(0, 2, 0.f);
  transform.set(1, 2, 0.f);
  transform.set(2, 2, 1.f);
  transform.set(3, 2, 0.f);

  EXPECT_FALSE(transform.IsFlat());

  transform.set(2, 0, 0.f);
  transform.set(2, 1, 0.f);
  transform.set(2, 3, 0.f);

  // Since the third column and row are both (0, 0, 1, 0), the transform is
  // flat.
  EXPECT_TRUE(transform.IsFlat());
}

// Another implementation of Preserves2dAxisAlignment that isn't as fast,
// good for testing the faster implementation.
static bool EmpiricallyPreserves2DAxisAlignment(const Xform3& transform) {
  Point3 p1(5.0f, 5.0f, 0.0f);
  Point3 p2(10.0f, 5.0f, 0.0f);
  Point3 p3(10.0f, 20.0f, 0.0f);
  Point3 p4(5.0f, 20.0f, 0.0f);

  Quad2 test_quad(
      Point2(p1.x, p1.y),
      Point2(p2.x, p2.y),
      Point2(p3.x, p3.y),
      Point2(p4.x, p4.y));
  EXPECT_TRUE(test_quad.IsRectilinear());

  p1 = transform.MapPoint(p1);
  p2 = transform.MapPoint(p2);
  p3 = transform.MapPoint(p3);
  p4 = transform.MapPoint(p4);

  Quad2 transformed_quad(
      Point2(p1.x, p1.y),
      Point2(p2.x, p2.y),
      Point2(p3.x, p3.y),
      Point2(p4.x, p4.y));
  return transformed_quad.IsRectilinear();
}

TEST(Xform3Test, Preserves2DAxisAlignment) {
  static const struct TestCase {
    float a; // row 1, column 1
    float b; // row 1, column 2
    float c; // row 2, column 1
    float d; // row 2, column 2
    bool expected;
  } test_cases[] = {
    { 3.f, 0.f,
      0.f, 4.f, true }, // basic case
    { 0.f, 4.f,
      3.f, 0.f, true }, // rotate by 90
    { 0.f, 0.f,
      0.f, 4.f, true }, // degenerate x
    { 3.f, 0.f,
      0.f, 0.f, true }, // degenerate y
    { 0.f, 0.f,
      3.f, 0.f, true }, // degenerate x + rotate by 90
    { 0.f, 4.f,
      0.f, 0.f, true }, // degenerate y + rotate by 90
    { 3.f, 4.f,
      0.f, 0.f, false },
    { 0.f, 0.f,
      3.f, 4.f, false },
    { 0.f, 3.f,
      0.f, 4.f, false },
    { 3.f, 0.f,
      4.f, 0.f, false },
    { 3.f, 4.f,
      5.f, 0.f, false },
    { 3.f, 4.f,
      0.f, 5.f, false },
    { 3.f, 0.f,
      4.f, 5.f, false },
    { 0.f, 3.f,
      4.f, 5.f, false },
    { 2.f, 3.f,
      4.f, 5.f, false },
  };

  Xform3 transform = Xform3::Identity();
  for (const TestCase& value : test_cases) {
    transform.SetIdentity();
    transform.set(0, 0, value.a);
    transform.set(0, 1, value.b);
    transform.set(1, 0, value.c);
    transform.set(1, 1, value.d);

    if (value.expected) {
      EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
      EXPECT_TRUE(transform.Preserves2DAxisAlignment());
    } else {
      EXPECT_FALSE(EmpiricallyPreserves2DAxisAlignment(transform));
      EXPECT_FALSE(transform.Preserves2DAxisAlignment());
    }
  }

  // Try the same test cases again, but this time make sure that other matrix
  // elements (except perspective) have entries, to test that they are ignored.
  for (const TestCase& value : test_cases) {
    transform.SetIdentity();
    transform.set(0, 0, value.a);
    transform.set(0, 1, value.b);
    transform.set(1, 0, value.c);
    transform.set(1, 1, value.d);

    transform.set(0, 2, 1.f);
    transform.set(0, 3, 2.f);
    transform.set(1, 2, 3.f);
    transform.set(1, 3, 4.f);
    transform.set(2, 0, 5.f);
    transform.set(2, 1, 6.f);
    transform.set(2, 2, 7.f);
    transform.set(2, 3, 8.f);

    if (value.expected) {
      EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
      EXPECT_TRUE(transform.Preserves2DAxisAlignment());
    } else {
      EXPECT_FALSE(EmpiricallyPreserves2DAxisAlignment(transform));
      EXPECT_FALSE(transform.Preserves2DAxisAlignment());
    }
  }

  // Try the same test cases again, but this time add perspective which is
  // always assumed to not-preserve axis alignment.
  for (const TestCase& value : test_cases) {
    transform.SetIdentity();
    transform.set(0, 0, value.a);
    transform.set(0, 1, value.b);
    transform.set(1, 0, value.c);
    transform.set(1, 1, value.d);

    transform.set(0, 2, 1.f);
    transform.set(0, 3, 2.f);
    transform.set(1, 2, 3.f);
    transform.set(1, 3, 4.f);
    transform.set(2, 0, 5.f);
    transform.set(2, 1, 6.f);
    transform.set(2, 2, 7.f);
    transform.set(2, 3, 8.f);
    transform.set(3, 0, 9.f);
    transform.set(3, 1, 10.f);
    transform.set(3, 2, 11.f);
    transform.set(3, 3, 12.f);

    EXPECT_FALSE(EmpiricallyPreserves2DAxisAlignment(transform));
    EXPECT_FALSE(transform.Preserves2DAxisAlignment());
  }

  // Try a few more practical situations to check precision
  transform.SetIdentity();
  transform.RotateAboutZAxis(Angle::DegreesToRadians(90.0));
  EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_TRUE(transform.Preserves2DAxisAlignment());

  transform.SetIdentity();
  transform.RotateAboutZAxis(Angle::DegreesToRadians(180.0));
  EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_TRUE(transform.Preserves2DAxisAlignment());

  transform.SetIdentity();
  transform.RotateAboutZAxis(Angle::DegreesToRadians(270.0));
  EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_TRUE(transform.Preserves2DAxisAlignment());

  transform.SetIdentity();
  transform.RotateAboutYAxis(Angle::DegreesToRadians(90.0));
  EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_TRUE(transform.Preserves2DAxisAlignment());

  transform.SetIdentity();
  transform.RotateAboutXAxis(Angle::DegreesToRadians(90.0));
  EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_TRUE(transform.Preserves2DAxisAlignment());

  transform.SetIdentity();
  transform.RotateAboutZAxis(Angle::DegreesToRadians(90.0));
  transform.RotateAboutYAxis(Angle::DegreesToRadians(90.0));
  EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_TRUE(transform.Preserves2DAxisAlignment());

  transform.SetIdentity();
  transform.RotateAboutZAxis(Angle::DegreesToRadians(90.0));
  transform.RotateAboutXAxis(Angle::DegreesToRadians(90.0));
  EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_TRUE(transform.Preserves2DAxisAlignment());

  transform.SetIdentity();
  transform.RotateAboutYAxis(Angle::DegreesToRadians(90.0));
  transform.RotateAboutZAxis(Angle::DegreesToRadians(90.0));
  EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_TRUE(transform.Preserves2DAxisAlignment());

  transform.SetIdentity();
  transform.RotateAboutZAxis(Angle::DegreesToRadians(45.0));
  EXPECT_FALSE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_FALSE(transform.Preserves2DAxisAlignment());

  // 3-d case; In 2d after an orthographic projection, this case does
  // preserve 2d axis alignment. But in 3d, it does not preserve axis
  // alignment.
  transform.SetIdentity();
  transform.RotateAboutYAxis(Angle::DegreesToRadians(45.0));
  EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_TRUE(transform.Preserves2DAxisAlignment());

  transform.SetIdentity();
  transform.RotateAboutXAxis(Angle::DegreesToRadians(45.0));
  EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_TRUE(transform.Preserves2DAxisAlignment());

  // Perspective cases.
  transform.SetIdentity();
  transform.ApplyPerspectiveDepth(10.0);
  transform.RotateAboutYAxis(Angle::DegreesToRadians(45.0));
  EXPECT_FALSE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_FALSE(transform.Preserves2DAxisAlignment());

  transform.SetIdentity();
  transform.ApplyPerspectiveDepth(10.0);
  transform.RotateAboutZAxis(Angle::DegreesToRadians(90.0));
  EXPECT_TRUE(EmpiricallyPreserves2DAxisAlignment(transform));
  EXPECT_TRUE(transform.Preserves2DAxisAlignment());
}

TEST(Xform3Test, BackFaceVisiblilityTolerance) {
  Xform3 backface_invisible = Xform3::Identity();
  backface_invisible.set(0, 3, 1.f);
  backface_invisible.set(3, 0, 1.f);
  backface_invisible.set(2, 0, 1.f);
  backface_invisible.set(3, 2, 1.f);

  // The transformation matrix has a determinant = 1 and cofactor33 = 0. So,
  // IsBackFaceVisible should return false.
  EXPECT_EQ(backface_invisible.GetDeterminant(), 1.f);
  EXPECT_FALSE(backface_invisible.IsBackFaceVisible());

  // Adding a noise to the transformsation matrix that is within the tolerance
  // (machine epsilon) should not change the result.
  float noise = Limits<float>::Epsilon;
  backface_invisible.set(0, 3, 1.f + noise);
  EXPECT_FALSE(backface_invisible.IsBackFaceVisible());

  // A noise that is more than the tolerance should change the result.
  backface_invisible.set(0, 3, 1.f + (2 * noise));
  EXPECT_TRUE(backface_invisible.IsBackFaceVisible());
}

} // namespace
} // namespace stp
