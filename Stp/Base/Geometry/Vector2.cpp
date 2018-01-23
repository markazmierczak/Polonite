// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Geometry/Vector2.h"

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
  return Hypot(x, y);
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
  return *this * (1 / Sqrt(lsqr));
}

bool Vector2::IsNormalized() const {
  return IsNear(GetLengthSquared(), 1.0, static_cast<double>(FLT_EPSILON));
}

Vector2 Floor(Vector2 v) {
  return Vector2(Floor(v.x), Floor(v.y));
}

Vector2 Ceil(Vector2 v) {
  return Vector2(Ceil(v.x), Ceil(v.y));
}

Vector2 Trunc(Vector2 v) {
  return Vector2(Trunc(v.x), Trunc(v.y));
}

Vector2 Round(Vector2 v) {
  return Vector2(Round(v.x), Round(v.y));
}

IntVector2 FloorToInt(Vector2 v) {
  return IntVector2(FloorToInt(v.x), FloorToInt(v.y));
}

IntVector2 CeilToInt(Vector2 v) {
  return IntVector2(CeilToInt(v.x), CeilToInt(v.y));
}

IntVector2 TruncToInt(Vector2 v) {
  return IntVector2(TruncToInt(v.x), TruncToInt(v.y));
}

IntVector2 RoundToInt(Vector2 v) {
  return IntVector2(RoundToInt(v.x), RoundToInt(v.y));
}

bool IsNear(const Vector2& lhs, const Vector2& rhs, float tolerance) {
  return IsNear(lhs.x, rhs.x, tolerance) &&
         IsNear(lhs.y, rhs.y, tolerance);
}

IntVector2 Lerp(const IntVector2& a, const IntVector2& b, double t) {
  return IntVector2(
      Lerp(a.x, b.x, t),
      Lerp(a.y, b.y, t));
}

Vector2 Lerp(const Vector2& a, const Vector2& b, double t) {
  return Vector2(
      Lerp(a.x, b.x, t),
      Lerp(a.y, b.y, t));
}

} // namespace stp
