// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Geometry/Ellipse.h"

#include "Base/Math/Math.h"
#include "Base/Text/Format.h"

namespace stp {

bool Ellipse::Contains(Point2 point) const {
  auto d = point - center;
  auto r = radii;
  return (d.x * d.x) / (r.x * r.x) + (d.y * d.y) / (r.y * r.y) <= 1;
}

void Ellipse::ToFormat(TextWriter& out, const StringSpan& opts) const {
  Format(out, center);
  out.Write(' ');
  out.WriteFloat(radii.x);
  out.Write('x');
  out.WriteFloat(radii.y);
}

} // namespace stp
