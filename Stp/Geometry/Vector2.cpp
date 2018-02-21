// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Vector2.h"

#include "Base/Io/TextWriter.h"
#include "Base/Math/FloatToInteger.h"

namespace stp {

int64_t IntVector2::GetLengthSquared() const {
  return static_cast<int64_t>(x) * x + static_cast<int64_t>(y) * y;
}

float IntVector2::GetLength() const {
  return Vector2(*this).GetLength();
}

void IntVector2::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out.Write('[');
  out.WriteInteger(x);
  out.Write(' ');
  out.WriteInteger(y);
  out.Write(']');
}

void Vector2::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out.Write('[');
  out.WriteFloat(x);
  out.Write(' ');
  out.WriteFloat(y);
  out.Write(']');
}

double Vector2::GetLengthSquared() const {
  return static_cast<double>(x) * x + static_cast<double>(y) * y;
}

float Vector2::GetLength() const {
  return mathHypot(x, y);
}

bool Vector2::TryScaleToLength(double new_length) {
  double old_length = GetLength();
  if (old_length <= Limits<float>::Epsilon)
    return false;
  operator*=(new_length / old_length);
  return true;
}

bool Vector2::TryNormalize() {
  return TryScaleToLength(1);
}

Vector2 Vector2::GetNormalizedOrThis() const {
  double lsqr = GetLengthSquared();
  if (lsqr <= Limits<double>::Epsilon)
    return *this;
  return *this * (1 / mathSqrt(lsqr));
}

bool Vector2::IsNormalized() const {
  return isNear(GetLengthSquared(), 1.0, static_cast<double>(FLT_EPSILON));
}

Vector2 mathFloor(Vector2 v) {
  return Vector2(mathFloor(v.x), mathFloor(v.y));
}

Vector2 mathCeil(Vector2 v) {
  return Vector2(mathCeil(v.x), mathCeil(v.y));
}

Vector2 mathTrunc(Vector2 v) {
  return Vector2(mathTrunc(v.x), mathTrunc(v.y));
}

Vector2 mathRound(Vector2 v) {
  return Vector2(mathRound(v.x), mathRound(v.y));
}

IntVector2 mathFloorToInt(Vector2 v) {
  return IntVector2(mathFloorToInt(v.x), mathFloorToInt(v.y));
}

IntVector2 mathCeilToInt(Vector2 v) {
  return IntVector2(mathCeilToInt(v.x), mathCeilToInt(v.y));
}

IntVector2 mathTruncToInt(Vector2 v) {
  return IntVector2(mathTruncToInt(v.x), mathTruncToInt(v.y));
}

IntVector2 mathRoundToInt(Vector2 v) {
  return IntVector2(mathRoundToInt(v.x), mathRoundToInt(v.y));
}

bool isNear(const Vector2& lhs, const Vector2& rhs, float tolerance) {
  return isNear(lhs.x, rhs.x, tolerance) &&
         isNear(lhs.y, rhs.y, tolerance);
}

IntVector2 lerp(const IntVector2& a, const IntVector2& b, double t) {
  return IntVector2(
      lerp(a.x, b.x, t),
      lerp(a.y, b.y, t));
}

Vector2 lerp(const Vector2& a, const Vector2& b, double t) {
  return Vector2(
      lerp(a.x, b.x, t),
      lerp(a.y, b.y, t));
}

} // namespace stp
