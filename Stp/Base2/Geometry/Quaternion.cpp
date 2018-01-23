// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Geometry/Quaternion.h"

#include "Base/Debug/Assert.h"
#include "Base/Io/TextWriter.h"
#include "Base/Math/Math.h"
#include "Base/Type/Limits.h"

namespace stp {

Quaternion Quaternion::FromAngleAxis(double radians, const Vector3& axis) {
  Vector3 axis_unit = axis;
  if (!axis_unit.TryNormalize())
    return Quaternion::Identity();
  return FromAngleAxisUnit(radians, axis_unit);
}

Quaternion Quaternion::FromAngleAxisUnit(double radians, const Vector3& axis) {
  ASSERT(axis.IsNormalized());
  double angle_2 = radians * 0.5;

  double sin_angle, cos_angle;
  SinCos(angle_2).Unpack(sin_angle, cos_angle);

  double w = cos_angle;
  double x = axis.x * sin_angle;
  double y = axis.y * sin_angle;
  double z = axis.z * sin_angle;
  return Quaternion(w, x, y, z);
}

double Quaternion::ToAngleAxis(Vector3* axis) const {
  double radians;
  double length_squared = GetLengthSquared();
  if (length_squared <= Limits<float>::Epsilon) {
    radians = 0;
    *axis = Vector3(1, 0, 0);
  } else {
    radians = 2 * Acos(w);
    double inv_length = 1 / Sqrt(length_squared);
    *axis = Vector3(x, y, z) * inv_length;
  }
  return radians;
}

void Quaternion::SetEulerAngles(double yaw, double pitch, double roll) {
  double sy, cy;
  double sp, cp;
  double sr, cr;

  SinCos(yaw   * 0.5).Unpack(sy, cy);
  SinCos(pitch * 0.5).Unpack(sp, cp);
  SinCos(roll  * 0.5).Unpack(sr, cr);

  w = cy * cr * cp + sy * sr * sp;
  x = cy * cr * sp - sy * sr * cp;
  y = sy * cr * cp + cy * sr * sp;
  z = cy * sr * cp - sy * cr * sp;
}

Quaternion::EulerAngles Quaternion::ToEulerAngles() const {
  double pitch = Atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y));
  double yaw = Asin(2 * (w * y - z * x));
  double roll = Atan2(2 * (w * z + x * y), 1 - 2 * (y * y + z * z));
  return EulerAngles { yaw, pitch, roll };
}

Quaternion Quaternion::FromRotationTo(const Vector3& from, const Vector3& to) {
  double dot = DotProduct(from, to);
  double norm = Sqrt(from.GetLengthSquared() * to.GetLengthSquared());
  double real = norm + dot;
  Vector3 axis;
  if (real < Limits<double>::Epsilon * norm) {
    real = 0.0f;
    axis = Abs(from.x) > Abs(from.z) ? Vector3(-from.y, from.x, 0) : Vector3(0, -from.z, from.y);
  } else {
    axis = CrossProduct(from, to);
  }
  return Quaternion(real, axis.x, axis.y, axis.z).GetNormalized();
}

static inline double Pow2(double x) { return x * x; }

double Quaternion::GetLengthSquared() const {
  return Pow2(w) + Pow2(x) + Pow2(y) + Pow2(z);
}

double Quaternion::GetLength() const {
  return Sqrt(GetLengthSquared());
}

bool Quaternion::Normalize() {
  double length_squared = GetLengthSquared();
  if (length_squared <= Limits<float>::Epsilon)
    return false;
  operator*=(1 / Sqrt(length_squared));
  return true;
}

Quaternion Quaternion::GetNormalized() const {
  double length_squared = GetLengthSquared();
  if (length_squared <= Limits<float>::Epsilon)
    return *this;
  return *this * (1 / Sqrt(length_squared));
}

bool Quaternion::TryGetInverted(Quaternion& out) const {
  double len = GetLengthSquared();
  if (len < Limits<float>::Epsilon)
    return false;

  double s = 1 / len;
  out = Quaternion(w * s, -x * s, -y * s, -z * s);
  return true;
}

void Quaternion::SetConcat(const Quaternion& lhs, const Quaternion& rhs) {
  double t0 = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
  double t1 = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
  double t2 = lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x;
  double t3 = lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w;

  w = t0; x = t1; y = t2; z = t3;
}

double DotProduct(const Quaternion& lhs, const Quaternion& rhs) {
  return lhs.w * rhs.w + lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, double t) {
  return (q1 * (1 - t) + q2 * t).GetNormalized();
}

// Taken from http://www.w3.org/TR/css3-transforms/.
Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, double t) {
  double dot = DotProduct(q1, q2);

  dot = Clamp(dot, -1.0, 1.0);

  constexpr double Epsilon = Limits<float>::Epsilon;
  if (IsNear(dot, 1.0, Epsilon) || IsNear(dot, -1.0, Epsilon))
    return q1;

  double denom = Sqrt(1.0 - dot * dot);
  double theta = Acos(dot);

  double spt, cpt;
  SinCos(t * theta).Unpack(spt, cpt);

  double w = spt * (1.0 / denom);

  double scale1 = cpt - dot * w;
  double scale2 = w;
  return q1 * scale1 + q2 * scale2;
}

bool IsNear(const Quaternion& lhs, const Quaternion& rhs, double tolerance) {
  return IsNear(lhs.w, rhs.w, tolerance) &&
         IsNear(lhs.x, rhs.x, tolerance) &&
         IsNear(lhs.y, rhs.y, tolerance) &&
         IsNear(lhs.z, rhs.z, tolerance);
}

void Quaternion::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out.Write('(');
  out.WriteFloat(w);
  out.Write(' ');
  out.WriteFloat(x);
  out.Write(' ');
  out.WriteFloat(y);
  out.Write(' ');
  out.WriteFloat(z);
  out.Write(')');
}

} // namespace stp
