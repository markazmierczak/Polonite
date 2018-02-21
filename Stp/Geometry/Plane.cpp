// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Plane.h"

#include "Base/Type/Limits.h"
#include "Geometry/Ray3.h"

namespace stp {

Plane::Plane(const Point3& origin, const Vector3& normal_) {
  ASSERT(normal_.IsNormalized());
  normal = normal_;
  distance = -DotProduct(normal_, origin);
}

bool Plane::operator==(const Plane& other) const {
  return other.normal == normal && other.distance == distance;
}

float Plane::GetDistanceToWithSign(const Point3& point) const {
  return DotProduct(normal, point) + distance;
}

float Plane::GetDistanceTo(const Point3& point) const {
  return mathAbs(GetDistanceToWithSign(point));
}

Plane::Side Plane::ClassifyPoint(const Point3& point) const {
  constexpr float Epsilon = Limits<float>::Epsilon;

  float distance = GetDistanceToWithSign(point);
  if (distance > Epsilon)
    return FrontSide;
  if (distance < -Epsilon)
    return BackSide;
  return NoSide;
}

Point3 Plane::ProjectPoint(const Point3& point) const {
  return point - normal * GetDistanceToWithSign(point);
}

bool Plane::IsParallelTo(const Plane& other) const {
  return !IntersectsWith(other);
}

bool Plane::IntersectsWith(const Plane& other, Ray3* ray) const {
  constexpr double Epsilon = Limits<double>::Epsilon;

  Vector3 cross_normals = CrossProduct(normal, other.normal);
  if (cross_normals.GetLengthSquared() < Epsilon)
    return false;

  if (ray) {
    double fn00 = normal.GetLength();
    double fn01 = DotProduct(normal, other.normal);
    double fn11 = other.normal.GetLength();
    double det = fn00 * fn11 - fn01 * fn01;
    ASSERT(mathAbs(det) > Epsilon);

    double invdet = 1.0 / det;
    float fc0 = (fn11 * -distance + fn01 * other.distance) * invdet;
    float fc1 = (fn00 * -other.distance + fn01 * distance) * invdet;

    Point3 origin = normal * fc0 + other.normal * fc1;
    *ray = Ray3(origin, cross_normals.GetNormalizedOrThis());
  }
  return true;
}

Vector3 Plane::ReflectVector(const Vector3& direction) const {
  ASSERT(direction.IsNormalized());
  float factor = 2 * DotProduct(normal, direction);
  return direction - normal * factor;
}

void Plane::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out.WriteAscii("n=");
  format(out, normal);
  out.WriteAscii(", z=");
  out.WriteFloat(distance);
}

bool isNear(const Plane& lhs, const Plane& rhs, float tolerance) {
  return isNear(lhs.normal, rhs.normal, tolerance) &&
         isNear(lhs.distance, rhs.distance, tolerance);
}

} // namespace stp
