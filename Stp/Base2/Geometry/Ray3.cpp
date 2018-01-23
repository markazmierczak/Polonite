// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Geometry/Ray3.h"

#include "Base/Geometry/Triangle3.h"
#include "Base/Text/Format.h"
#include "Base/Type/Limits.h"

namespace stp {

Point3 Ray3::ProjectPoint(const Point3& point) const {
  Vector3 offset = point - origin;
  return origin + direction * DotProduct(offset, direction);
}

double Ray3::GetDistanceToPoint(const Point3& point) const {
  return (point - ProjectPoint(point)).GetLength();
}

Point3 Ray3::GetClosestPoint(const Ray3& other) const {
  // Algorithm based on http://paulbourke.net/geometry/pointlineplane/
  Vector3 p13 = origin - other.origin;
  Vector3 p43 = other.direction;
  Vector3 p21 = direction;

  float d1343 = DotProduct(p13, p43);
  float d4321 = DotProduct(p43, p21);
  float d1321 = DotProduct(p13, p21);
  float d4343 = DotProduct(p43, p43);
  float d2121 = DotProduct(p21, p21);

  float d = d2121 * d4343 - d4321 * d4321;
  if (Abs(d) <= Limits<float>::Epsilon)
    return origin;

  float n = d1343 * d4321 - d1321 * d4343;
  float a = n / d;
  return origin + direction * a;
}

bool Ray3::IntersectsTriangle(
    const Triangle3& triangle,
    bool culling,
    float* distance, Vector3* normal) const {
  const Point3& p0 = triangle.p;
  const Point3& p1 = triangle.q;
  const Point3& p2 = triangle.r;

  // Based on Fast, Minimum Storage Ray/Triangle Intersection by
  // Muller & Trumbore.
  // http://www.graphics.cornell.edu/pubs/1997/MT97.pdf
  // Calculate edge vectors
  Vector3 edge1(p1 - p0);
  Vector3 edge2(p2 - p0);

  // Calculate determinant & check back-facing.
  Vector3 p = CrossProduct(direction, edge2);
  float det = DotProduct(edge1, p);
  if (culling) {
    if (det <= Limits<float>::Epsilon)
      return false;
  } else {
    if (Abs(det) <= Limits<float>::Epsilon)
      return false;
  }

  // Distance to plane, < 0 means ray behind the plane.
  Vector3 t(origin - p0);
  float u = DotProduct(t, p);
  if (u < 0 || u > det)
    return false;
  Vector3 q(CrossProduct(t, edge1));
  float v = DotProduct(direction, q);
  if (v < 0 || u + v > det)
    return false;

  if (distance)
    *distance = DotProduct(edge2, q) / det;
  if (normal)
    *normal = CrossProduct(edge1, edge2);

  return true;
}

bool Ray3::IntersectsTriangle(const Triangle3& triangle, float* distance) const {
  return IntersectsTriangle(triangle, true, distance, nullptr);
}

void Ray3::ToFormat(TextWriter& out, const StringSpan& opts) const {
  Format(out, origin);
  out.WriteAscii("-dir-");
  Format(out, direction);
}

} // namespace stp
