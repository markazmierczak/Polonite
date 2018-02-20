// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Vector3.h"

#include "Base/Io/TextWriter.h"
#include "Base/Math/FloatToInteger.h"
#include "Base/Type/Limits.h"
#include "Geometry/Angle.h"

namespace stp {

int64_t IntVector3::GetLengthSquared() const {
  return static_cast<int64_t>(x) * x +
         static_cast<int64_t>(y) * y +
         static_cast<int64_t>(z) * z;
}

double IntVector3::GetLength() const {
  return Sqrt(static_cast<double>(GetLengthSquared()));
}

double Vector3::GetLengthSquared() const {
  return static_cast<double>(x) * x +
         static_cast<double>(y) * y +
         static_cast<double>(z) * z;
}

double Vector3::GetLength() const {
  return Sqrt(GetLengthSquared());
}

Vector3 Floor(const Vector3& v) {
  return Vector3(Floor(v.x), Floor(v.y), Floor(v.z));
}

Vector3 Ceil(const Vector3& v) {
  return Vector3(Ceil(v.x), Ceil(v.y), Ceil(v.z));
}

Vector3 Trunc(const Vector3& v) {
  return Vector3(Trunc(v.x), Trunc(v.y), Trunc(v.z));
}

Vector3 Round(const Vector3& v) {
  return Vector3(Round(v.x), Round(v.y), Round(v.z));
}

IntVector3 FloorToInt(const Vector3& v) {
  return IntVector3(FloorToInt(v.x), FloorToInt(v.y), FloorToInt(v.z));
}

IntVector3 CeilToInt(const Vector3& v) {
  return IntVector3(CeilToInt(v.x), CeilToInt(v.y), CeilToInt(v.z));
}

IntVector3 TruncToInt(const Vector3& v) {
  return IntVector3(TruncToInt(v.x), TruncToInt(v.y), TruncToInt(v.z));
}

IntVector3 RoundToInt(const Vector3& v) {
  return IntVector3(RoundToInt(v.x), RoundToInt(v.y), RoundToInt(v.z));
}

bool Vector3::TryScaleToLength(double new_length) {
  double old_length = GetLength();
  if (old_length <= Limits<float>::Epsilon)
    return false;
  operator*=(new_length / old_length);
  return true;
}

bool Vector3::TryNormalize() {
  return TryScaleToLength(1);
}

Vector3 Vector3::GetNormalizedOrThis() const {
  double lsqr = GetLengthSquared();
  if (lsqr <= Limits<double>::Epsilon)
    return *this;
  return *this * (1 / Sqrt(lsqr));
}

bool Vector3::IsNormalized() const {
  return isNear(GetLengthSquared(), 1.0, static_cast<double>(Limits<float>::Epsilon));
}

bool isNear(const Vector3& lhs, const Vector3& rhs, float tolerance) {
  return isNear(lhs.x, rhs.x, tolerance) &&
         isNear(lhs.y, rhs.y, tolerance) &&
         isNear(lhs.z, rhs.z, tolerance);
}

IntVector3 min(const IntVector3& lhs, const IntVector3& rhs) {
  return IntVector3(
      min(lhs.x, rhs.x),
      min(lhs.y, rhs.y),
      min(lhs.z, rhs.z));
}

IntVector3 max(const IntVector3& lhs, const IntVector3& rhs) {
  return IntVector3(
      max(lhs.x, rhs.x),
      max(lhs.y, rhs.y),
      max(lhs.z, rhs.z));
}

Vector3 min(const Vector3& lhs, const Vector3& rhs) {
  return Vector3(
      min(lhs.x, rhs.x),
      min(lhs.y, rhs.y),
      min(lhs.z, rhs.z));
}

Vector3 max(const Vector3& lhs, const Vector3& rhs) {
  return Vector3(
      max(lhs.x, rhs.x),
      max(lhs.y, rhs.y),
      max(lhs.z, rhs.z));
}

IntVector3 lerp(const IntVector3& a, const IntVector3& b, double t) {
  return IntVector3(
      lerp(a.x, b.x, t),
      lerp(a.y, b.y, t),
      lerp(a.z, b.z, t));
}

Vector3 lerp(const Vector3& a, const Vector3& b, double t) {
  return Vector3(
      lerp(a.x, b.x, t),
      lerp(a.y, b.y, t),
      lerp(a.z, b.z, t));
}

Vector3 CrossProduct(const Vector3& lhs, const Vector3& rhs) {
  float x = lhs.y * rhs.z - lhs.z * rhs.y;
  float y = lhs.z * rhs.x - lhs.x * rhs.z;
  float z = lhs.x * rhs.y - lhs.y * rhs.x;
  return Vector3(x, y, z);
}

double Angle::BetweenInRadians(const Vector3& base, const Vector3& other) {
  return Acos(DotProduct(base, other) / base.GetLength() / other.GetLength());
}

double Angle::ClockwiseBetweenInRadians(
    const Vector3& base, const Vector3& other, const Vector3& normal) {
  double radians = BetweenInRadians(base, other);

  Vector3 cross = CrossProduct(base, other);

  // If the dot product of this cross product is normal, it means that the
  // shortest angle between |base| and |other| was counterclockwise with respect
  // to the surface represented by |normal| and this angle must be reversed.
  if (DotProduct(cross, normal) > 0.0f)
    radians = FullInRadians - radians;

  return radians;
}

void IntVector3::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out << '[' << x << ' ' << y << ' ' << z << ']';
}

void Vector3::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out << '[' << x << ' ' << y << ' ' << z << ']';
}

} // namespace stp
