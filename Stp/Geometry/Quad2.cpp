// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Geometry/Quad2.h"

#include "Base/Debug/Assert.h"
#include "Base/Text/FormatMany.h"
#include "Geometry/Affine.h"
#include "Geometry/Bounds2.h"
#include "Geometry/Triangle2.h"
#include "Geometry/Xform2.h"

namespace stp {

bool Quad2::IsRectilinear(float tol) const {
  return (IsNear(p[0].x, p[1].x, tol) &&
          IsNear(p[1].y, p[2].y, tol) &&
          IsNear(p[2].x, p[3].x, tol) &&
          IsNear(p[3].y, p[0].y, tol)) ||
         (IsNear(p[0].y, p[1].y, tol) &&
          IsNear(p[1].x, p[2].x, tol) &&
          IsNear(p[2].y, p[3].y, tol) &&
          IsNear(p[3].x, p[0].x, tol));
}

bool Quad2::IsCounterClockwise() const {
  // This math computes the signed area of the quad. Positive area
  // indicates the quad is clockwise; negative area indicates the quad is
  // counter-clockwise. Note carefully: this is backwards from conventional
  // math because our geometric space uses screen coordiantes with y-axis
  // pointing downards.
  // Reference: http://mathworld.wolfram.com/PolygonArea.html.
  // The equation can be written:
  // Signed area = determinant1 + determinant2 + determinant3 + determinant4
  // In practise, Refactoring the computation of adding determinants so that
  // reducing the number of operations. The equation is:
  // Signed area = element1 + element2 - element3 - element4

  float p24 = p[1].y - p[3].y;
  float p31 = p[2].y - p[0].y;

  // Up-cast to double so this cannot overflow.
  double element1 = static_cast<double>(p[0].x) * p24;
  double element2 = static_cast<double>(p[1].x) * p31;
  double element3 = static_cast<double>(p[2].x) * p24;
  double element4 = static_cast<double>(p[3].x) * p31;

  return element1 + element2 < element3 + element4;
}

bool Quad2::contains(const Point2& point) const {
  return Triangle2(p[0], p[1], p[2]).contains(point)
      || Triangle2(p[0], p[2], p[3]).contains(point);
}

void Quad2::Scale(float x_scale, float y_scale) {
  for (Point2& point : p)
    point.Scale(x_scale, y_scale);
}

void Quad2::RotateCorners(int times) {
  int index = times & 3;
  Point2 tmp = p[index];
  for (int i = 0; i < 4; ++i) {
    int next_index = (index + 1) & 3;
    p[index] = p[next_index];
    index = next_index;
  }
  p[index] = tmp;
}

void Quad2::Transform(const Affine& affine) {
  affine.MapPoints(p, p, 4);
}

void Quad2::Transform(const Xform2& xform) {
  xform.MapPoints(p, p, 4);
}

Bounds2 Quad2::GetBounds() const {
  return Bounds2::Enclose(p, 4);
}

void Quad2::operator+=(Vector2 rhs) {
  for (Point2& point : p)
    point += rhs;
}

void Quad2::operator-=(Vector2 rhs) {
  for (Point2& point : p)
    point -= rhs;
}

void Quad2::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out.Write('(');
  for (int i = 0; i < 4; ++i) {
    if (i != 0)
      out.Write(',');
    format(out, p[i]);
  }
  out.Write(')');
}

} // namespace stp
