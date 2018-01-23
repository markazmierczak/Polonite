// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_TRIANGLE3_H_
#define STP_BASE_GEOMETRY_TRIANGLE3_H_

#include "Base/Geometry/Vector3.h"

namespace stp {

struct BASE_EXPORT Triangle3 {
  Triangle3() {}
  Triangle3(const Point3& p, const Point3& q, const Point3& r) : p(p), q(q), r(r) {}

  // Gives the center point of the triangle (geometric average of vertices).
  Point3 GetCentroid() const;

  // Gives the vector normal to the triangle (result is not normalized).
  Vector3 GetFaceNormal() const { return CrossProduct(q - p, r - q); }

  Point3 p;
  Point3 q;
  Point3 r;
};

} // namespace stp

#endif // STP_BASE_GEOMETRY_TRIANGLE3_H_
