// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Affine.h"

#include "Base/Test/GTest.h"
#include "Geometry/Angle.h"
#include "Geometry/Bounds2.h"

namespace stp {

TEST(AffineTest, Translate) {
  Affine t(Affine::SkipInit);
  t.SetTranslate(Vector2(-10, 9));
  EXPECT_EQ(Point2(-9, 10), t.MapPoint(Point2(1, 1)));

  t = Affine(1, 3, 2, 4, 5, 6);
  Affine rt1(1, 3, 2, 4, 55, 116);
  Affine rt2(t);
  t.Translate(Vector2(10, 20));
  EXPECT_EQ(rt1, t);
  t.Translate(Vector2(-10, -20));
  EXPECT_EQ(rt2, t);
}

TEST(AffineTest, Scale) {
  Affine t(1, 3, 2, 4, 5, 6);
  Affine rt1(t);
  Affine rt2(10, 30, 40, 80, 5, 6);
  t.Scale(10,  20);
  EXPECT_EQ(t, rt2);
  t.Scale(1.0f / 10.0f, 1.0f / 20.0f);
  EXPECT_EQ(t, rt1);
}

TEST(AffineTest, Rotate) {
  Affine t(Affine::SkipInit);
  t.SetRotate(Angle::DegreesToRadians(-90.0));
  EXPECT_EQ(Point2(9, 10), t.MapPoint(Point2(-10, 9)));

  t = Affine(1, 3, 2, 4, 5, 6);
  Affine rt1(t);
  Affine rt2(-1, -3, -2, -4, 5, 6);
  t.Rotate(Angle::DegreesToRadians(180.0));
  EXPECT_TRUE(IsNear(t, rt2, 0.001f));
  t.Rotate(Angle::DegreesToRadians(-180.0));
  EXPECT_TRUE(IsNear(t, rt1, 0.001f));
}

TEST(AffineTest, SkewX) {
  Affine t = Affine::Identity();
  t.SkewX(Angle::DegreesToRadians(-45.0));
  EXPECT_EQ(Point2(0, 0), t.MapPoint(Point2(0, 0)));
  EXPECT_EQ(Point2(9, -9), t.MapPoint(Point2(0, -9)));
  EXPECT_EQ(Point2(7, -3), t.MapPoint(Point2(4, -3)));
}

TEST(AffineTest, InitWithIdentity) {
  Affine t(Affine::SkipInit);
  t.SetIdentity();
  EXPECT_TRUE(t.IsIdentity());
  EXPECT_TRUE(t.IsTranslate());
  EXPECT_TRUE(t.Preserves2DAxisAlignment());
  t.Translate(Vector2(1, 1));
  EXPECT_FALSE(t.IsIdentity());
  EXPECT_TRUE(t.IsTranslate());
  EXPECT_TRUE(t.Preserves2DAxisAlignment());
  t.Scale(3, 2);
  EXPECT_FALSE(t.IsIdentity());
  EXPECT_FALSE(t.IsTranslate());
  EXPECT_TRUE(t.Preserves2DAxisAlignment());
}

TEST(AffineTest, MapPoint) {
  Affine t(Affine::SkipInit);
  t.SetIdentity();
  Point2 p(1, -2);
  EXPECT_EQ(p, t.MapPoint(p));
  t.Translate(3, -1);
  EXPECT_EQ(Point2(4, -3), t.MapPoint(p));
  t.Scale(2, 2);
  EXPECT_EQ(Point2(5, -5), t.MapPoint(p));

  t.SetRotate(Angle::DegreesToRadians(90.0));
  EXPECT_TRUE(IsNear(Point2(2, 1), t.MapPoint(p), 0.0001f));
}

TEST(AffineTest, Concat) {
  Affine t2(1, 3, 2, 4, 5, 6);
  Affine t1(2, 4, 3, 5, 6, 7);

  Affine result1x2(11, 19, 16, 28, 34, 57);
  Affine result2x1(10, 22, 13, 29, 25, 52);

  Affine product12(Affine::SkipInit);
  Affine product21(Affine::SkipInit);
  product12.SetConcat(t1, t2);
  product21.SetConcat(t2, t1);

  EXPECT_EQ(result1x2, product12);
  EXPECT_EQ(result2x1, product21);

  product12 = t1;
  product12.Concat(t2);
  product21 = t2;
  product21.Concat(t1);
  EXPECT_EQ(result1x2, product12);
  EXPECT_EQ(result2x1, product21);

  product12 = t2;
  product12.PostConcat(t1);
  product21 = t1;
  product21.PostConcat(t2);
  EXPECT_EQ(result1x2, product12);
  EXPECT_EQ(result2x1, product21);
}

TEST(AffineTest, SetBoundsToBounds) {
  Bounds2 src(-10, -10, 20, 20);
  Bounds2 dst(-30, 50, 60, 110);
  Affine mat(Affine::SkipInit);
  mat.SetBoundsToBounds(src, dst);

  Bounds2 expected(-24, 54, 54, 106);
  src.Inset(2, 2);
  EXPECT_EQ(expected, mat.MapBounds(src));
}

TEST(AffineTest, RectStaysRect) {
  static const struct {
    float m00, m01, m10, m11;
    bool stays_rect;
  } tests[] = {
    { 0, 0, 0, 0, false },
    { 0, 0, 0, 1, false },
    { 0, 0, 1, 0, false },
    { 0, 0, 1, 1, false },
    { 0, 1, 0, 0, false },
    { 0, 1, 0, 1, false },
    { 0, 1, 1, 0, true },
    { 0, 1, 1, 1, false },
    { 1, 0, 0, 0, false },
    { 1, 0, 0, 1, true },
    { 1, 0, 1, 0, false },
    { 1, 0, 1, 1, false },
    { 1, 1, 0, 0, false },
    { 1, 1, 0, 1, false },
    { 1, 1, 1, 0, false },
    { 1, 1, 1, 1, false }
  };

  for (const auto& test : tests) {
    Affine m(Affine::SkipInit);
    m = Affine(test.m00, test.m01, test.m10, test.m11, 0, 0);
    EXPECT_EQ(test.stays_rect, m.Preserves2DAxisAlignment());
  }
}

TEST(AffineTest, GetInverted) {
  Affine mat(Affine::SkipInit);
  Affine iden1(Affine::SkipInit);
  Affine iden2(Affine::SkipInit);
  mat.SetTranslate(1, 1);
  Affine inverse(Affine::SkipInit);
  EXPECT_TRUE(mat.GetInverted(inverse));
  iden1.SetConcat(mat, inverse);
  EXPECT_TRUE(iden1.IsIdentity());

  mat.SetScale(2, 4);
  EXPECT_TRUE(mat.GetInverted(inverse));
  iden1.SetConcat(mat, inverse);
  EXPECT_TRUE(iden1.IsIdentity());

  mat.SetScale(0.5f, 2);
  EXPECT_TRUE(mat.GetInverted(inverse));
  iden1.SetConcat(mat, inverse);
  EXPECT_TRUE(iden1.IsIdentity());

  mat.SetScale(3, 5, 20, 0);
  mat.PostRotate(25.0);
  EXPECT_TRUE(mat.IsInvertible());
  EXPECT_TRUE(mat.GetInverted(inverse));
  iden1.SetConcat(mat, inverse);
  EXPECT_TRUE(IsNear(iden1, Affine::Identity(), 0.0001f));
  iden2.SetConcat(inverse, mat);
  EXPECT_TRUE(IsNear(iden2, Affine::Identity(), 0.0001f));

  mat.SetScale(0, 1);
  EXPECT_FALSE(mat.IsInvertible());
  EXPECT_FALSE(mat.GetInverted(inverse));
  mat.SetScale(1, 0);
  EXPECT_FALSE(mat.IsInvertible());
  EXPECT_FALSE(mat.GetInverted(inverse));
}

TEST(AffineTest, ConcatOperator) {
  Affine a(Affine::SkipInit);
  a.SetTranslate(10, 20);

  Affine b(Affine::SkipInit);
  b.SetScale(3, 5);

  Affine expected(Affine::SkipInit);
  expected.SetConcat(a,b);

  EXPECT_EQ(expected, a * b);
}

} // namespace stp
