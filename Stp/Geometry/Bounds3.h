// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_BOUNDS3_H_
#define STP_BASE_GEOMETRY_BOUNDS3_H_

#include "Base/Type/Variable.h"
#include "Geometry/Vector3.h"

namespace stp {

class Xform3;

struct BASE_EXPORT IntBounds3 {
  IntBounds3() {}
  IntBounds3(const IntPoint3& min, const IntPoint3& max) : min(min), max(max) {}

  IntVector3 GetSize() const { return max - min; }

  IntPoint3 GetCenterPoint() const { return (min + max) / 2; }

  int GetWidth() const { return max.x - min.x; }
  int GetHeight() const { return max.y - min.y; }
  int GetDepth() const { return max.z - min.z; }

  bool isEmpty() const { return min.x >= max.x || min.y >= max.y || min.z >= max.z; }

  bool contains(IntPoint3 point) const { return contains(point.x, point.y, point.z); }
  bool contains(int x, int y, int z) const;

  void Sort();
  IntBounds3 GetSorted() const WARN_UNUSED_RESULT { auto s = *this; s.Sort(); return s; }

  void operator+=(const IntVector3& offset) { min += offset; max += offset; }
  void operator-=(const IntVector3& offset) { min -= offset; max -= offset; }

  IntBounds3 operator+(const IntVector3& offset) const;
  IntBounds3 operator-(const IntVector3& offset) const;

  bool operator==(const IntBounds3& other) const { return min == other.min && max == other.max; }
  bool operator!=(const IntBounds3& other) const { return !operator==(other); }

  IntPoint3 min;
  IntPoint3 max;
};

struct BASE_EXPORT Bounds3 {
  Bounds3() {}
  Bounds3(const Point3& min, const Point3& max) : min(min), max(max) {}

  Vector3 GetSize() const { return max - min; }

  Point3 GetCenterPoint() const { return (min + max) * 0.5f; }

  float GetWidth() const { return max.x - min.x; }
  float GetHeight() const { return max.y - min.y; }
  float GetDepth() const { return max.z - min.z; }

  bool isEmpty() const { return min.x >= max.x || min.y >= max.y || min.z >= max.z; }

  bool contains(Point3 point) const { return contains(point.x, point.y, point.z); }
  bool contains(float x, float y, float z) const;

  void Sort();
  Bounds3 GetSorted() const WARN_UNUSED_RESULT { auto s = *this; s.Sort(); return s; }

  void Transform(const Xform3& xform);

  void operator+=(const Vector3& offset) { min += offset; max += offset; }
  void operator-=(const Vector3& offset) { min -= offset; max -= offset; }

  Bounds3 operator+(const Vector3& offset) const { return Bounds3(min + offset, max + offset); }
  Bounds3 operator-(const Vector3& offset) const { return Bounds3(min - offset, max - offset); }

  bool operator==(const Bounds3& other) const { return min == other.min && max == other.max; }
  bool operator!=(const Bounds3& other) const { return !operator==(other); }

  Point3 min;
  Point3 max;
};

inline bool IntBounds3::contains(int x, int y, int z) const {
  return (unsigned)(x - min.x) <= (unsigned)(max.x - min.x) &&
         (unsigned)(y - min.y) <= (unsigned)(max.y - min.y) &&
         (unsigned)(z - min.z) <= (unsigned)(max.z - min.z);
}

inline bool Bounds3::contains(float x, float y, float z) const {
  return min.x <= x && x <= max.x && min.y <= y && y <= max.y && min.z <= z && z <= max.z;
}

inline void IntBounds3::Sort() {
  if (min.x > max.x)
    swap(min.x, max.x);
  if (min.y > max.y)
    swap(min.y, max.y);
  if (min.z > max.z)
    swap(min.z, max.z);
}

inline void Bounds3::Sort() {
  if (min.x > max.x)
    swap(min.x, max.x);
  if (min.y > max.y)
    swap(min.y, max.y);
  if (min.z > max.z)
    swap(min.z, max.z);
}

inline IntBounds3 IntBounds3::operator+(const IntVector3& offset) const {
  return IntBounds3(min + offset, max + offset);
}

inline IntBounds3 IntBounds3::operator-(const IntVector3& offset) const {
  return IntBounds3(min - offset, max - offset);
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_BOUNDS3_H_
