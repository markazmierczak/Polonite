// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_RAY3_H_
#define STP_BASE_GEOMETRY_RAY3_H_

#include "Base/Debug/Assert.h"
#include "Geometry/Vector3.h"

namespace stp {

struct Triangle3;

// Infinite straight line in 3D space.
struct BASE_EXPORT Ray3 {
  Ray3() {}

  Ray3(const Point3& origin, const Vector3& direction)
      : origin(origin), direction(direction) {
    ASSERT(direction.IsNormalized());
  }

  Point3 ProjectPoint(const Point3& point) const;

  double GetDistanceToPoint(const Point3& point) const;

  Point3 GetClosestPoint(const Ray3& other) const;

  // Distance to the triangle from origin will be stored in |distance|.
  // |normal| will be filled with triangle's normal.
  // |normal| and |distance| are only altered on positive result of intersection.
  bool IntersectsTriangle(
      const Triangle3& triangle,
      bool culling,
      float* distance, Vector3* normal) const;

  bool IntersectsTriangle(const Triangle3& triangle, float* distance = nullptr) const;

  void Transform(const Xform3& xform);

  bool operator==(const Ray3& o) const;
  bool operator!=(const Ray3& rhs) const { return !operator==(rhs); }

  Point3 origin;
  Vector3 direction;
};

inline bool Ray3::operator==(const Ray3& o) const {
  return origin == o.origin && direction == o.direction;
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_RAY3_H_
