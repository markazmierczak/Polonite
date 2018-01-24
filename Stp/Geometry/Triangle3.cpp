// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Triangle3.h"

namespace stp {

Point3 Triangle3::GetCentroid() const {
  Point3 sum = Point3(
      p.x + q.x + r.x,
      p.y + q.y + r.y,
      p.z + q.z + r.z);

  return sum * (1.f / 3);
}

} // namespace stp
