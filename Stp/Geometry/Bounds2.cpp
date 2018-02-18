// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Bounds2.h"

#include "Base/Text/FormatMany.h"

namespace stp {

Bounds2 RoundOut(const Bounds2& b) {
  return Bounds2(Floor(b.min), Ceil(b.max));
}

Bounds2 RoundIn(const Bounds2& b) {
  return Bounds2(Ceil(b.min), Ceil(b.max));
}

Bounds2 RoundNearest(const Bounds2& b) {
  return Bounds2(Round(b.min), Round(b.max));
}

IntBounds2 RoundOutToInt(const Bounds2& b) {
  return IntBounds2(FloorToInt(b.min), CeilToInt(b.max));
}

IntBounds2 RoundInToInt(const Bounds2& b) {
  return IntBounds2(CeilToInt(b.min), FloorToInt(b.max));
}

IntBounds2 RoundNearestToInt(const Bounds2& b) {
  return IntBounds2(RoundToInt(b.min), RoundToInt(b.max));
}

bool IntBounds2::Intersects(const IntBounds2& lhs, const IntBounds2& rhs) {
  auto d1 = lhs.min - rhs.max;
  auto d2 = rhs.min - lhs.max;
  return d1.x <= 0 && d1.y <= 0 && d2.x <= 0 && d2.y <= 0;
}

bool Bounds2::Intersects(const Bounds2& lhs, const Bounds2& rhs) {
  auto d1 = lhs.min - rhs.max;
  auto d2 = rhs.min - lhs.max;
  return d1.x <= 0 && d1.y <= 0 && d2.x <= 0 && d2.y <= 0;
}

bool IntBounds2::TryIntersect(const IntBounds2& other) {
  auto minx = max(min.x, other.min.x);
  auto maxx = min(max.x, other.max.x);
  auto miny = max(min.y, other.min.y);
  auto maxy = min(max.y, other.max.y);
  if (minx >= maxx || miny >= maxy)
    return false;

  min = IntPoint2(minx, miny);
  max = IntPoint2(maxx, maxy);
  return true;
}

bool Bounds2::TryIntersect(const Bounds2& other) {
  auto minx = max(min.x, other.min.x);
  auto maxx = min(max.x, other.max.x);
  auto miny = max(min.y, other.min.y);
  auto maxy = min(max.y, other.max.y);
  if (minx >= maxx || miny >= maxy)
    return false;

  min = Point2(minx, miny);
  max = Point2(maxx, maxy);
  return true;
}

void IntBounds2::Unite(const IntBounds2& other) {
  if (other.IsEmpty())
    return;

  if (IsEmpty()) {
    *this = other;
  } else {
    min.x = min(min.x, other.min.x);
    min.y = min(min.y, other.min.y);
    max.x = max(max.x, other.max.x);
    max.y = max(max.y, other.max.y);
  }
}

void Bounds2::Unite(const Bounds2& other) {
  if (other.IsEmpty())
    return;

  if (IsEmpty()) {
    *this = other;
  } else {
    min.x = min(min.x, other.min.x);
    min.y = min(min.y, other.min.y);
    max.x = max(max.x, other.max.x);
    max.y = max(max.y, other.max.y);
  }
}

template<typename T, typename P>
static inline T OfPointsTemplate(const P points[], int count) {
  if (count == 0)
    return T();

  auto rx = points[0].x;
  auto ry = points[0].y;
  auto rr = rx;
  auto rb = ry;

  for (int i = 0; i < count; ++i) {
    auto x = points[i].x;
    auto y = points[i].y;
    rx = min(rx, x);
    ry = min(ry, y);
    rr = max(rr, x);
    rb = max(rb, y);
  }
  return T(rx, ry, rr, rb);
}

IntBounds2 IntBounds2::Enclose(const IntPoint2 points[], int count) {
  return OfPointsTemplate<IntBounds2>(points, count);
}

Bounds2 Bounds2::Enclose(const Point2 points[], int count) {
  return OfPointsTemplate<Bounds2>(points, count);
}

void IntBounds2::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out << min << max << ' ' << GetWidth() << 'x' << GetHeight();
}

void Bounds2::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out << min << max << ' ' << GetWidth() << 'x' << GetHeight();
}

} // namespace stp
