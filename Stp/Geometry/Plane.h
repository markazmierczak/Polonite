// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_PLANE_H_
#define STP_BASE_GEOMETRY_PLANE_H_

#include "Base/Debug/Assert.h"
#include "Geometry/Vector3.h"

namespace stp {

struct Ray3;

// Plane in Hessian normal form. This is obtained from general equation of plane:
//  ax + by + cz + d = 0
// Plane include every point for which the following equation is true:
// dot(normal_, point) + distance_ = 0
class BASE_EXPORT Plane {
 public:
  enum Side {
    NoSide, // The plane itself.
    FrontSide,
    BackSide,
    BothSides
  };

  Plane(const Vector3& normal, float distance)
      : normal(normal), distance(distance) {
    ASSERT(normal.IsNormalized());
  }

  Plane(const Point3& origin, const Vector3& normal);

  float GetDistanceTo(const Point3& point) const;

  // Same as above but can return negative values depending on which side the point is.
  float GetDistanceToWithSign(const Point3& point) const;

  Side ClassifyPoint(const Point3& point) const;

  Point3 ProjectPoint(const Point3& point) const;

  // Returns true if |other| is parallel to the plane.
  bool IsParallelTo(const Plane& other) const;

  bool IntersectsWith(const Plane& other, Ray3* ray = nullptr) const;

  // |direction| must be normalized.
  Vector3 ReflectVector(const Vector3& direction) const;

  bool operator==(const Plane& o) const;
  bool operator!=(const Plane& o) const { return !operator==(o); }

  Vector3 normal; // this normal must be always normalized
  float distance;
};

BASE_EXPORT bool isNear(const Plane& lhs, const Plane& rhs, float tolerance);

} // namespace stp

#endif // STP_BASE_GEOMETRY_PLANE_H_
