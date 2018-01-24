// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Plane.h"

#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"
#include "Geometry/Ray3.h"

namespace stp {

TEST(Plane3dTest, Constructors) {
  Plane plane_xy(Vector3(0, 0, 1), 1);
  Plane plane_xy_calculated(Point3(0, 0, -1), Vector3(0, 0, 1));
  EXPECT_EQ(plane_xy, plane_xy_calculated);
  EXPECT_TRUE(IsNear(plane_xy, plane_xy_calculated, Limits<float>::Epsilon));
}

TEST(Plane3dTest, GetDistanceTo) {
  Plane plane(Vector3(0, 0, 1), 0);
  EXPECT_EQ(0, plane.GetDistanceTo(Point3(0, 0, 0)));
  EXPECT_EQ(1 + 2, plane.GetDistanceTo(Point3(1, 2, 3)));
  EXPECT_EQ(1 + 2, plane.GetDistanceTo(Point3(-1, -2, -3)));
  EXPECT_EQ(1 + 2, plane.GetDistanceToWithSign(Point3(1, 2, 3)));
  EXPECT_EQ(-1 -2, plane.GetDistanceToWithSign(Point3(-1, -2, -3)));
}

TEST(Plane3dTest, ClassifyPoint) {
  Plane plane(Vector3(0, 1, 0), 2);
  EXPECT_EQ(Plane::NoSide, plane.ClassifyPoint(Point3(1, -2, 3)));
  EXPECT_EQ(Plane::BackSide, plane.ClassifyPoint(Point3(10, -3, 1)));
  EXPECT_EQ(Plane::FrontSide, plane.ClassifyPoint(Point3(1, 3, 10)));
}

TEST(Plane3dTest, ProjectPoint) {
  Plane plane(Vector3(0, 1, 0), -2);
  EXPECT_EQ(Point3(1, 2, 3), plane.ProjectPoint(Point3(1, 10, 3)));
}

TEST(Plane3dTest, IntersectsWith) {
  Plane plane(Vector3(0, 1, 0), -2);
  EXPECT_TRUE(plane.IsParallelTo(Plane(Vector3(0, 1, 0), 2)));
  EXPECT_FALSE(plane.IsParallelTo(Plane(Vector3(0, 0, 1), 2)));

  Plane plane_xy(Vector3(0, 0, -1), 1);
  Ray3 intersection;
  bool result = plane.IntersectsWith(plane_xy, &intersection);
  EXPECT_TRUE(result);
  EXPECT_EQ(Ray3(Point3(0, 2, 1), Vector3(-1, 0, 0)), intersection);
}

} // namespace stp
