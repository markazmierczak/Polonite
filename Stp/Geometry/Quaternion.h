// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_QUATERNION_H_
#define STP_BASE_GEOMETRY_QUATERNION_H_

#include "Geometry/Angle.h"
#include "Geometry/Vector3.h"

namespace stp {

struct BASE_EXPORT Quaternion {
  enum SkipInitTag { SkipInit };
  enum InitWithIdentityTag { InitWithIdentity };

  explicit Quaternion(SkipInitTag) {}

  constexpr explicit Quaternion(InitWithIdentityTag)
      : w(1), x(0), y(0), z(0) {}

  constexpr Quaternion(double w, double x, double y, double z)
      : w(w), x(x), y(y), z(z) {}

  static constexpr Quaternion Identity() { return Quaternion(InitWithIdentity); }

  void SetIdentity() { *this = Identity(); }
  bool IsIdentity() const { return *this == Identity(); }

  static constexpr Quaternion Zero() { return Quaternion(0, 0, 0, 0); }
  bool IsZero() const { return w == 0 && x == 0 && y == 0 && z == 0; }

  static Quaternion FromAngleAxis(double radians, const Vector3& axis);
  // Expects axis to be normalized.
  static Quaternion FromAngleAxisUnit(double radians, const Vector3& axis);

  double ToAngleAxis(Vector3* axis) const;

  static Quaternion FromEulerAngles(double yaw, double pitch, double roll);

  void SetEulerAngles(double yaw, double pitch, double roll);

  struct EulerAngles {
    double yaw;
    double pitch;
    double roll;

    void Unpack(double& out_yaw, double& out_pitch, double& out_roll) const {
      out_yaw = yaw; out_pitch = pitch; out_roll = roll;
    }
  };

  explicit Quaternion(EulerAngles ea);
  EulerAngles ToEulerAngles() const;

  static Quaternion FromRotationTo(const Vector3& from, const Vector3& to);

  void SetConcat(const Quaternion& lhs, const Quaternion& rhs);
  void Concat(const Quaternion& rhs) { SetConcat(*this, rhs); }

  Quaternion GetInversed() const { return Quaternion(w, -x, -y, -z); }

  Quaternion operator+(const Quaternion& o) const;
  Quaternion operator-(const Quaternion& o) const;

  void operator+=(const Quaternion& o);
  void operator-=(const Quaternion& o);

  Quaternion operator*(double scale) const;
  Quaternion operator/(double scale) const;

  void operator*=(double scale);
  void operator/=(double scale);

  Quaternion operator*(const Quaternion& rhs) const;
  void operator*=(const Quaternion& rhs) { Concat(rhs); }

  double GetLengthSquared() const;
  double GetLength() const;

  bool Normalize();
  Quaternion GetNormalized() const WARN_UNUSED_RESULT;

  Quaternion GetConjugated() const { return Quaternion(w, -x, -y, -z); }

  bool tryGetInverted(Quaternion& out) const WARN_UNUSED_RESULT;

  bool operator==(const Quaternion& o) const;
  bool operator!=(const Quaternion& o) const { return !operator==(o); }

  friend Quaternion operator*(double scale, const Quaternion& q) { return q * scale; }

  double w;
  double x;
  double y;
  double z;
};

BASE_EXPORT Quaternion lerp(const Quaternion& q1, const Quaternion& q2, double t);

// Spherical linear interpolation between two quaternions
BASE_EXPORT Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, double t);

BASE_EXPORT bool isNear(const Quaternion& lhs, const Quaternion& rhs, double tolerance);

BASE_EXPORT double DotProduct(const Quaternion& lhs, const Quaternion& rhs);

inline Quaternion Quaternion::FromEulerAngles(double yaw, double pitch, double roll) {
  Quaternion out(SkipInit);
  out.SetEulerAngles(yaw, pitch, roll);
  return out;
}

inline Quaternion::Quaternion(EulerAngles ea) {
  SetEulerAngles(ea.yaw, ea.pitch, ea.roll);
}

inline Quaternion Quaternion::operator+(const Quaternion& o) const {
  return Quaternion(w + o.w, x + o.x, y + o.y, z + o.z);
}

inline Quaternion Quaternion::operator-(const Quaternion& o) const {
  return Quaternion(w - o.w, x - o.x, y - o.y, z - o.z);
}

inline void Quaternion::operator+=(const Quaternion& o) {
  w += o.w;
  x += o.x;
  y += o.y;
  z += o.z;
}

inline void Quaternion::operator-=(const Quaternion& o) {
  w -= o.w;
  x -= o.x;
  y -= o.y;
  z -= o.z;
}

inline Quaternion Quaternion::operator*(double scale) const {
  return Quaternion(w * scale, x * scale, y * scale, z * scale);
}

inline Quaternion Quaternion::operator/(double scale) const {
  return Quaternion(w / scale, x / scale, y / scale, z / scale);
}

inline void Quaternion::operator*=(double scale) {
  w *= scale;
  x *= scale;
  y *= scale;
  z *= scale;
}

inline void Quaternion::operator/=(double scale) {
  w /= scale;
  x /= scale;
  y /= scale;
  z /= scale;
}

inline Quaternion Quaternion::operator*(const Quaternion& rhs) const {
  Quaternion r(SkipInit);
  r.SetConcat(*this, rhs);
  return r;
}

inline bool Quaternion::operator==(const Quaternion& o) const {
  return w == o.w && x == o.x && y == o.y && z == o.z;
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_QUATERNION_H_
