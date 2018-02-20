// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Bounds3.h"

#include "Geometry/Xform3.h"

namespace stp {

void Bounds3::Transform(const Xform3& xform) {
  min = xform.MapPoint(min);
  max = xform.MapPoint(max);
}

void IntBounds3::ToFormat(TextWriter& out, const StringSpan& opts) const {
  formatMany(out, "{} {} {}x{}x{}", min, max, GetWidth(), GetHeight(), GetDepth());
}

void Bounds3::ToFormat(TextWriter& out, const StringSpan& opts) const {
  formatMany(out, "{} {} {}x{}x{}", min, max, GetWidth(), GetHeight(), GetDepth());
}

} // namespace stp
