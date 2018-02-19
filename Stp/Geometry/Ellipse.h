// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_ELLIPSE_H_
#define STP_BASE_GEOMETRY_ELLIPSE_H_

#include "Base/Math/Math.h"
#include "Base/Math/MathConstants.h"
#include "Geometry/Bounds2.h"

namespace stp {

struct BASE_EXPORT Ellipse {
  Ellipse() {}

  Ellipse(float center_x, float center_y, float radius_x, float radius_y)
      : center(center_x, center_y), radii(radius_x, radius_y) {}

  Ellipse(const Point2& center, float radius)
      : center(center), radii(radius, radius) {}

  Ellipse(const Point2& center, const Vector2& radii)
      : center(center), radii(radii) {}

  static Ellipse FromBounds(const Bounds2& bounds);

  bool isEmpty() const { return radii.x <= 0 || radii.y <= 0; }

  double GetArea() const { return MathConstants<double>::Pi * radii.x * radii.y; }

  Bounds2 GetBounds() const { return Bounds2(center - radii, center + radii); }

  bool contains(Point2 point) const;
  bool contains(float x, float y) const { return contains(Point2(x, y)); }

  void Scale(float x_scale, float y_scale) { radii.Scale(x_scale, y_scale); }

  void operator*=(float scale) { Scale(scale, scale); }
  Ellipse operator*(float scale) const { return Ellipse(center, radii * scale); }

  void operator+=(const Vector2& d) { center += d; }
  void operator-=(const Vector2& d) { center -= d; }

  Ellipse operator+(const Vector2& d) const { return Ellipse(center + d, radii); }
  Ellipse operator-(const Vector2& d) const { return Ellipse(center - d, radii); }

  Point2 center;
  Vector2 radii;
};

inline Ellipse Ellipse::FromBounds(const Bounds2& bounds) {
  return Ellipse(bounds.GetCenterPoint(), Vector2(bounds.GetWidth(), bounds.GetHeight()) * 0.5f);
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_ELLIPSE_H_
