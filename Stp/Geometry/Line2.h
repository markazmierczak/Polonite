// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_LINE2_H_
#define STP_BASE_GEOMETRY_LINE2_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Limits.h"
#include "Geometry/Vector2.h"

namespace stp {

struct Bounds2;
struct IntBounds2;
class Affine;

struct BASE_EXPORT IntLine2 {
  IntLine2() {}

  IntLine2(const IntPoint2& p1, const IntPoint2& p2) : p1(p1), p2(p2) {}
  IntLine2(int x1, int y1, int x2, int y2) : p1(x1, y1), p2(x2, y2) {}

  bool IsEmpty() const { return p2 == p1; }

  IntBounds2 GetBounds() const;

  // Reverses direction of the line.
  IntLine2 operator-() const { return IntLine2(p2, p1); }

  bool IsVertical() const { return GetDelta().x == 0; }
  bool IsHorizontal() const { return GetDelta().y == 0; }

  IntVector2 GetDelta() const { return p2 - p1; }

  int x1() const { return p1.x; }
  int y1() const { return p1.y; }
  int x2() const { return p2.x; }
  int y2() const { return p2.y; }

  bool operator==(const IntLine2& o) const { return p1 == o.p1 && p2 == o.p2; }
  bool operator!=(const IntLine2& o) const { return !operator==(o); }

  void ToFormat(TextWriter& out, const StringSpan& opts) const;

  IntPoint2 p1;
  IntPoint2 p2;
};

struct BASE_EXPORT Line2 {
  Line2() {}

  Line2(const Point2& p1, const Point2& p2) : p1(p1), p2(p2) {}
  Line2(float x1, float y1, float x2, float y2) : p1(x1, y1), p2(x2, y2) {}

  explicit Line2(const IntLine2& line) : p1(line.p1), p2(line.p2) {}

  static bool Intersects(
      const Line2& a, const Line2& b,
      Point2* at_point = nullptr) WARN_UNUSED_RESULT;

  static bool Intersects(
      const Line2& line, const Bounds2& bounds,
      Line2* out_clipped = nullptr) WARN_UNUSED_RESULT;

  void Intersect(const Bounds2& bounds);

  float GetDistanceTo(Point2 p) const;

  bool IsEmpty() const { return p2 == p1; }

  float GetLength() const { return GetDelta().GetLength(); }

  Bounds2 GetBounds() const;

  // Reverses direction of the line.
  Line2 operator-() const { return Line2(p2, p1); }

  void operator+=(const Vector2& v) { p1 += v; p2 += v; }
  void operator-=(const Vector2& v) { p1 -= v; p2 -= v; }

  Line2 operator+(const Vector2& rhs) const { Line2 r = *this; r += rhs; return r; }
  Line2 operator-(const Vector2& rhs) const { Line2 r = *this; r -= rhs; return r; }

  bool IsVertical(float tolerance = Limits<float>::Epsilon) const;
  bool IsHorizontal(float tolerance = Limits<float>::Epsilon) const;

  Point2 GetCenterPoint() const { return (p1 + p2) / 2; }

  // https://en.wikipedia.org/wiki/Slope
  // Check !IsVertical() first.
  float GetSlope() const;

  Vector2 GetDelta() const { return p2 - p1; }

  float x1() const { return p1.x; }
  float y1() const { return p1.y; }
  float x2() const { return p2.x; }
  float y2() const { return p2.y; }

  bool operator==(const Line2& o) const { return p1 == o.p1 && p2 == o.p2; }
  bool operator!=(const Line2& o) const { return !operator==(o); }

  void Transform(const Affine& affine);

  void ToFormat(TextWriter& out, const StringSpan& opts) const;

  double DistanceToSquared(Point2 p) const;

  Point2 p1;
  Point2 p2;
};

inline bool Line2::IsVertical(float tolerance) const {
  return IsNear(x1(), x2(), tolerance);
}

inline bool Line2::IsHorizontal(float tolerance) const {
  return IsNear(y1(), y2(), tolerance);
}

inline float Line2::GetSlope() const {
  Vector2 d = GetDelta();
  return d.y / d.x;
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_LINE2_H_
