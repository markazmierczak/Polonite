// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_TRIANGLE2_H_
#define STP_BASE_GEOMETRY_TRIANGLE2_H_

#include "Geometry/Vector2.h"

namespace stp {

struct BASE_EXPORT Triangle2 {
  Triangle2() {}
  Triangle2(const Point2& p, const Point2& q, const Point2& r) : p(p), q(q), r(r) {}

  // Gives the center point of the triangle (geometric average of vertices).
  Point2 GetCentroid() const;

  bool contains(Point2 point) { return contains(point.x, point.y); }
  bool contains(float px, float py);

  Point2 p;
  Point2 q;
  Point2 r;
};

} // namespace stp

#endif // STP_BASE_GEOMETRY_TRIANGLE2_H_
