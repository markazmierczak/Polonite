// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_VECTOR2_H_
#define STP_BASE_GEOMETRY_VECTOR2_H_

#include "Base/Math/Abs.h"
#include "Base/Text/FormatFwd.h"

namespace stp {

struct IntVector2;
struct Vector2;
typedef IntVector2 IntPoint2;
typedef Vector2 Point2;

struct BASE_EXPORT IntVector2 {
  static constexpr IntVector2 Zero() { return IntVector2(); }
  static constexpr IntPoint2 Origin() { return IntPoint2(); }

  constexpr IntVector2() : x(0), y(0) {}
  constexpr IntVector2(int x, int y) : x(x), y(y) {}

  bool IsZero() const { return x == 0 && y == 0; }
  bool IsOrigin() const { return x == 0 && y == 0; }

  // Cheaper to compute than Length() - useful when you want to compare
  // relative lengths of different vectors without needing the actual lengths.
  int64_t GetLengthSquared() const;

  float GetLength() const;

  IntVector2 operator-() const { return IntVector2(-x, -y); }

  void operator+=(const IntVector2& d) { x += d.x; y += d.y; }
  void operator-=(const IntVector2& d) { x -= d.x; y -= d.y; }

  IntVector2 operator+(const IntVector2& rhs) const { return IntVector2(x + rhs.x, y + rhs.y); }
  IntVector2 operator-(const IntVector2& rhs) const { return IntVector2(x - rhs.x, y - rhs.y); }

  IntVector2 operator*(int factor) const { return IntVector2(x * factor, y * factor); }
  IntVector2 operator/(int factor) const { return IntVector2(x / factor, y / factor); }

  Vector2 operator*(float factor) const;
  Vector2 operator/(float factor) const;

  bool operator==(const IntVector2& o) const { return x == o.x && y == o.y; }
  bool operator!=(const IntVector2& o) const { return !operator==(o); }

  int x;
  int y;
};

struct BASE_EXPORT Vector2 {
  static constexpr Vector2 Zero() { return Vector2(); }
  static constexpr Point2 Origin() { return Point2(); }

  constexpr Vector2() : x(0), y(0) {}
  constexpr Vector2(float x, float y) : x(x), y(y) {}

  explicit Vector2(const IntVector2& v) : x(v.x), y(v.y) {}

  bool IsZero() const { return x == 0 && y == 0; }
  bool IsOrigin() const { return x == 0 && y == 0; }

  double GetLengthSquared() const;

  float GetLength() const;

  void Scale(float x_scale, float y_scale) { x *= x_scale; y *= y_scale; }
  Vector2 GetScaled(float x_scale, float y_scale) const;

  // Scale the vector to have the specified length. If the original length is
  // degenerately small (nearly zero), set it to zero and return false,
  // otherwise return true.
  bool TryScaleToLength(double length);

  // Normalizes the vector. Intentionally does not check whether vector is
  // already normalized. If vector's length is zero, nothing is changed and
  // false is returned.
  bool TryNormalize();
  Vector2 GetNormalizedOrThis() const WARN_UNUSED_RESULT;

  bool IsNormalized() const;

  Vector2 operator-() const { return Vector2(-x, -y); }

  void operator+=(const Vector2& d) { x += d.x; y += d.y; }
  void operator-=(const Vector2& d) { x -= d.x; y -= d.y; }

  Vector2 operator+(const Vector2& rhs) const { return Vector2(x + rhs.x, y + rhs.y); }
  Vector2 operator-(const Vector2& rhs) const { return Vector2(x - rhs.x, y - rhs.y); }

  void operator*=(float scale) { x *= scale; y *= scale; }
  void operator/=(float scale) { x /= scale; y /= scale; }

  Vector2 operator*(float scale) const { return Vector2(x * scale, y * scale); }
  Vector2 operator/(float scale) const { return Vector2(x / scale, y / scale); }

  bool operator==(const Vector2& o) const { return x == o.x && y == o.y; }
  bool operator!=(const Vector2& o) const { return !operator==(o); }

  const float* AsFloats() const { return &x; }
  float* AsFloats() { return &x; }

  float x;
  float y;
};

inline IntVector2 Abs(const IntVector2& v) { return IntVector2(Abs(v.x), Abs(v.y)); }
inline Vector2 Abs(const Vector2& v) { return Vector2(Abs(v.x), Abs(v.y)); }

inline IntVector2 Min(const IntVector2& lhs, const IntVector2& rhs) {
  return IntVector2(Min(lhs.x, rhs.x), Min(lhs.y, rhs.y));
}
inline IntVector2 Max(const IntVector2& lhs, const IntVector2& rhs) {
  return IntVector2(Max(lhs.x, rhs.x), Max(lhs.y, rhs.y));
}

inline Vector2 Min(const Vector2& lhs, const Vector2& rhs) {
  return Vector2(Min(lhs.x, rhs.x), Min(lhs.y, rhs.y));
}
inline Vector2 Max(const Vector2& lhs, const Vector2& rhs) {
  return Vector2(Max(lhs.x, rhs.x), Max(lhs.y, rhs.y));
}

BASE_EXPORT IntVector2 Lerp(const IntVector2& a, const IntVector2& b, double t);

BASE_EXPORT Vector2 Lerp(const Vector2& a, const Vector2& b, double t);

BASE_EXPORT bool IsNear(const Vector2& a, const Vector2& b, float tolerance);

BASE_EXPORT Vector2 Floor(Vector2 v);
BASE_EXPORT Vector2 Ceil(Vector2 v);
BASE_EXPORT Vector2 Trunc(Vector2 v);
BASE_EXPORT Vector2 Round(Vector2 v);

BASE_EXPORT IntVector2 FloorToInt(Vector2 v);
BASE_EXPORT IntVector2 CeilToInt(Vector2 v);
BASE_EXPORT IntVector2 TruncToInt(Vector2 v);
BASE_EXPORT IntVector2 RoundToInt(Vector2 v);

inline int CrossProduct(const IntVector2& lhs, const IntVector2& rhs) {
  return lhs.x * rhs.y - lhs.y * rhs.x;
}

inline int DotProduct(const IntVector2& lhs, const IntVector2& rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

inline float CrossProduct(const Vector2& lhs, const Vector2& rhs) {
  return lhs.x * rhs.y - lhs.y * rhs.x;
}

inline float DotProduct(const Vector2& lhs, const Vector2& rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

inline Vector2 Vector2::GetScaled(float x_scale, float y_scale) const {
  return Vector2(x * x_scale, y * y_scale);
}

inline Vector2 IntVector2::operator*(float factor) const {
  return Vector2(x * factor, y * factor);
}
inline Vector2 IntVector2::operator/(float factor) const {
  return Vector2(x / factor, y / factor);
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_VECTOR2_H_
