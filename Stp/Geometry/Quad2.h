// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_QUAD2_H_
#define STP_BASE_GEOMETRY_QUAD2_H_

#include "Base/Type/Limits.h"
#include "Geometry/Vector2.h"

namespace stp {

struct Bounds2;
class Affine;
class Xform2;

// A Quad is defined by four corners, allowing it to have edges that are not
// axis-aligned, unlike a Rect.
struct BASE_EXPORT Quad2 {
  Quad2() = default;

  Quad2(const Point2& p0, const Point2& p1, const Point2& p2, const Point2& p3)
      : p { p0, p1, p2, p3 } {}

  // Returns true if the quad is an axis-aligned rectangle.
  bool IsRectilinear(float tolerance = Limits<float>::Epsilon) const;

  // This assumes that the quad is convex, and that no three points are collinear.
  bool IsCounterClockwise() const;

  bool contains(const Point2& point) const;

  void Scale(float x_scale, float y_scale);

  // Realigns the corners in the quad by rotating them n corners.
  void RotateCorners(int times);

  void Transform(const Affine& affine);
  void Transform(const Xform2& xform);

  Bounds2 GetBounds() const;

  void operator+=(Vector2 rhs);
  void operator-=(Vector2 rhs);

  Quad2 operator+(const Vector2& rhs) const { Quad2 r = *this; r += rhs; return r; }
  Quad2 operator-(const Vector2& rhs) const { Quad2 r = *this; r -= rhs; return r; }

  void operator*=(float s) { Scale(s, s); }

  bool operator==(const Quad2& o) const;
  bool operator!=(const Quad2& o) const { return !operator==(o); }

  Point2 p[4];
};

inline bool Quad2::operator==(const Quad2& o) const {
  return p[0] == o.p[0] && p[1] == o.p[1] && p[2] == o.p[2] && p[3] == o.p[3];
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_QUAD2_H_
