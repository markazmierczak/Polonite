// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Geometry/Triangle2.h"

namespace stp {

Point2 Triangle2::GetCentroid() const {
  Point2 sum = Point2(
      p.x + q.x + r.x,
      p.y + q.y + r.y);

  return sum * (1.f / 3);
}

bool Triangle2::Contains(float px, float py) {
  // Compute the barycentric coordinates (u, v, w) of |point| relative to the
  // triangle (r1, r2, r3) by the solving the system of equations:
  //   1) point = u * r1 + v * r2 + w * r3
  //   2) u + v + w = 1
  // This algorithm comes from Christer Ericson's Real-Time Collision Detection.

  Vector2 drp = p - r;
  Vector2 drq = q - r;
  Vector2 drin = Point2(px, py) - r;

  float denom = drq.y * drp.x - drq.x * drp.y;
  float u = (drq.y * drin.x - drq.x * drin.y) / denom;
  float v = (drp.x * drin.y - drp.y * drin.x) / denom;
  float w = 1 - u - v;

  // Use the barycentric coordinates to test if |point| is inside the
  // triangle (r1, r2, r2).
  return u >= 0 && v >= 0 && w >= 0;
}

} // namespace stp
