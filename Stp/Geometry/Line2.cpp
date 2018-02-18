// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Line2.h"

#include "Base/Math/Math.h"
#include "Base/Text/FormatMany.h"
#include "Geometry/Affine.h"
#include "Geometry/Bounds2.h"

namespace stp {

IntBounds2 IntLine2::GetBounds() const {
  return IntBounds2(p1, p2).GetSorted();
}

Bounds2 Line2::GetBounds() const {
  return Bounds2(p1, p2).GetSorted();
}

double Line2::DistanceToSquared(Point2 p) const {
  Vector2 v = p2 - p1;
  Vector2 rel = p - p1;

  double dot = DotProduct(v, rel);
  double projected = dot * dot / v.GetLengthSquared();
  double len_squared = rel.GetLengthSquared() - projected;
  return max(len_squared, 0.0);
}

float Line2::GetDistanceTo(Point2 p) const {
  return Sqrt(DistanceToSquared(p));
}

bool Line2::Intersects(const Line2& a, const Line2& b, Point2* at_point) {
  float p0_x = a.x1();
  float p0_y = a.y1();
  float p1_x = a.x2();
  float p1_y = a.y2();

  float p2_x = b.x1();
  float p2_y = b.y1();
  float p3_x = b.x2();
  float p3_y = b.y2();

  float s1_x = p1_x - p0_x;
  float s1_y = p1_y - p0_y;
  float s2_x = p3_x - p2_x;
  float s2_y = p3_y - p2_y;

  float s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
  float t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

  if (!(s >= 0 && s <= 1 && t >= 0 && t <= 1))
    return false;

  if (at_point)
    *at_point = Point2(p0_x + (t * s1_x), p0_y + (t * s1_y));

  return true;
}

static inline bool NestedLT(float a, float b, float dim) {
  return a <= b && (a < b || dim > 0);
}

template<typename T>
static T PinUnsorted(T value, T limit0, T limit1) {
  if (limit1 < limit0) {
    swap(limit0, limit1);
  }
  // now the limits are sorted
  ASSERT(limit0 <= limit1);

  if (value < limit0) {
    value = limit0;
  } else if (value > limit1) {
    value = limit1;
  }
  return value;
}

// Return X coordinate of intersection with horizontal line at Y.
static float SectWithHorizontal(const Line2& line, float Y) {
  if (line.IsHorizontal())
    return line.GetCenterPoint().x;
  // Need the extra precision so we don't compute a value that exceeds
  // our original limits.
  double X0 = line.x1();
  double Y0 = line.y1();
  double X1 = line.x2();
  double Y1 = line.y2();
  double result = X0 + ((double)Y - Y0) * (X1 - X0) / (Y1 - Y0);

  // The computed X value might still exceed [X0..X1] due to quantum flux
  // when the doubles were added and subtracted, so we have to pin the
  // answer :(
  return (float)PinUnsorted(result, X0, X1);
}

// Return Y coordinate of intersection with vertical line at X.
static float SectWithVertical(const Line2& line, float X) {
  if (line.IsVertical())
    return line.GetCenterPoint().y;
  // Need the extra precision so we don't compute a value that exceeds
  // our original limits.
  double X0 = line.x1();
  double Y0 = line.y1();
  double X1 = line.x2();
  double Y1 = line.y2();
  double result = Y0 + ((double)X - X0) * (Y1 - Y0) / (X1 - X0);
  return static_cast<float>(result);
}

bool Line2::Intersects(const Line2& line, const Bounds2& clip, Line2* out_clipped) {
  Bounds2 bounds = line.GetBounds();
  if (clip.Contains(bounds)) {
    if (out_clipped && out_clipped != &line)
      *out_clipped = line;
    return true;
  }
  // Check for no overlap, and only permit coincident edges if the line
  // and the edge are colinear.
  if (NestedLT(bounds.max.x, clip.min.x, bounds.GetWidth()) ||
      NestedLT(clip.max.x, bounds.min.x, bounds.GetWidth()) ||
      NestedLT(bounds.max.y, clip.min.y, bounds.GetHeight()) ||
      NestedLT(clip.max.y, bounds.min.y, bounds.GetHeight())) {
    return false;
  }

  unsigned index0;
  unsigned index1;

  if (line.y1() < line.y2()) {
    index0 = 0;
    index1 = 1;
  } else {
    index0 = 1;
    index1 = 0;
  }

  Point2 tmp[2] = { line.p1, line.p2 };

  // now compute Y intersections
  if (tmp[index0].y < clip.min.y)
    tmp[index0] = Point2(SectWithHorizontal(line, clip.min.y), clip.min.y);

  if (tmp[index1].y > clip.max.y)
    tmp[index1] = Point2(SectWithHorizontal(line, clip.max.y), clip.max.y);

  if (tmp[0].x < tmp[1].x) {
    index0 = 0;
    index1 = 1;
  } else {
    index0 = 1;
    index1 = 0;
  }

  // check for quick-reject in X again, now that we may have been chopped
  if ((tmp[index1].x <= clip.min.x || tmp[index0].x >= clip.max.x) &&
      tmp[index0].x < tmp[index1].x) {
    // only reject if we have a non-zero width
    return false;
  }

  if (out_clipped) {
    if (tmp[index0].x < clip.min.x)
      tmp[index0] = Point2(clip.min.x, SectWithVertical(line, clip.min.x));
    if (tmp[index1].x > clip.max.x)
      tmp[index1] = Point2(clip.max.x, SectWithVertical(line, clip.max.x));
    *out_clipped = Line2(tmp[0], tmp[1]);
    ASSERT(clip.Contains(out_clipped->GetBounds()));
  }
  return true;
}

void Line2::Intersect(const Bounds2& bounds) {
  ignoreResult(Intersects(*this, bounds, this));
}

void Line2::Transform(const Affine& affine) {
  Point2 points[2] = { p1, p2 };
  affine.MapPoints(points, points, 2);
  p1 = points[0];
  p2 = points[1];
}

void IntLine2::ToFormat(TextWriter& out, const StringSpan& opts) const {
  format(out, p1);
  out.Write(' ');
  format(out, p2);
}

void Line2::ToFormat(TextWriter& out, const StringSpan& opts) const {
  format(out, p1);
  out.Write(' ');
  format(out, p2);
}

} // namespace stp
