// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_BOUNDS2_H_
#define STP_BASE_GEOMETRY_BOUNDS2_H_

#include "Base/Geometry/Vector2.h"
#include "Base/Type/Variable.h"

namespace stp {

struct BASE_EXPORT IntBounds2 {
  IntBounds2() {}
  IntBounds2(const IntPoint2& min, const IntPoint2& max) : min(min), max(max) {}

  IntBounds2(int min_x, int min_y, int max_x, int max_y)
      : min(min_x, min_y), max(max_x, max_y) {}

  IntVector2 GetSize() const { return max - min; }

  IntPoint2 GetCenterPoint() const { return (min + max) / 2; }

  int GetPerimeter() const;

  int GetWidth() const { return max.x - min.x; }
  int GetHeight() const { return max.y - min.y; }

  bool IsEmpty() const { return min.x >= max.x || min.y >= max.y; }

  bool Contains(IntPoint2 point) const { return Contains(point.x, point.y); }
  bool Contains(int x, int y) const;

  bool Contains(const IntBounds2& other) const;

  static bool Intersects(const IntBounds2& lhs, const IntBounds2& rhs);

  bool TryIntersect(const IntBounds2& other);
  void Unite(const IntBounds2& other);

  void Sort();
  IntBounds2 GetSorted() const WARN_UNUSED_RESULT { auto s = *this; s.Sort(); return s; }

  static IntBounds2 Enclose(const IntPoint2 points[], int count);

  void Inset(int dx, int dy);
  void Outset(int dx, int dy) { Inset(-dx, -dy); }

  void operator+=(const IntVector2& offset) { min += offset; max += offset; }
  void operator-=(const IntVector2& offset) { min -= offset; max -= offset; }

  IntBounds2 operator+(const IntVector2& offset) const;
  IntBounds2 operator-(const IntVector2& offset) const;

  bool operator==(const IntBounds2& other) const { return min == other.min && max == other.max; }
  bool operator!=(const IntBounds2& other) const { return !operator==(other); }

  void ToFormat(TextWriter& out, const StringSpan& opts) const;

  IntPoint2 min;
  IntPoint2 max;
};

struct BASE_EXPORT Bounds2 {
  Bounds2() {}
  Bounds2(const Point2& min, const Point2& max) : min(min), max(max) {}

  Bounds2(float min_x, float min_y, float max_x, float max_y)
      : min(min_x, min_y), max(max_x, max_y) {}

  Vector2 GetSize() const { return max - min; }

  Point2 GetCenterPoint() const { return (min + max) * 0.5f; }

  float GetPerimeter() const;

  float GetWidth() const { return max.x - min.x; }
  float GetHeight() const { return max.y - min.y; }

  bool IsEmpty() const { return min.x >= max.x || min.y >= max.y; }

  bool Contains(Point2 point) const { return Contains(point.x, point.y); }
  bool Contains(float x, float y) const;

  bool Contains(const Bounds2& other) const;

  static bool Intersects(const Bounds2& lhs, const Bounds2& rhs);

  bool TryIntersect(const Bounds2& other);
  void Unite(const Bounds2& other);

  void Sort();
  Bounds2 GetSorted() const WARN_UNUSED_RESULT { auto s = *this; s.Sort(); return s; }

  static Bounds2 Enclose(const Point2 points[], int count);

  void Inset(float dx, float dy);
  void Outset(float dx, float dy) { Inset(-dx, -dy); }

  void operator+=(const Vector2& offset) { min += offset; max += offset; }
  void operator-=(const Vector2& offset) { min -= offset; max -= offset; }

  Bounds2 operator+(const Vector2& offset) const { return Bounds2(min + offset, max + offset); }
  Bounds2 operator-(const Vector2& offset) const { return Bounds2(min - offset, max - offset); }

  bool operator==(const Bounds2& other) const { return min == other.min && max == other.max; }
  bool operator!=(const Bounds2& other) const { return !operator==(other); }

  void ToFormat(TextWriter& out, const StringSpan& opts) const;

  Point2 min;
  Point2 max;
};

// RoundIn()/RoundInToInt() may return unsorted rectangle - check with IsEmpty().
BASE_EXPORT Bounds2 RoundOut(const Bounds2& b);
BASE_EXPORT Bounds2 RoundIn(const Bounds2& b);
BASE_EXPORT Bounds2 RoundNearest(const Bounds2& b);

BASE_EXPORT IntBounds2 RoundOutToInt(const Bounds2& b);
BASE_EXPORT IntBounds2 RoundInToInt(const Bounds2& b);
BASE_EXPORT IntBounds2 RoundNearestToInt(const Bounds2& b);

inline int IntBounds2::GetPerimeter() const {
  return 2 * ((max.x - min.x) + (max.y - min.y));
}

inline float Bounds2::GetPerimeter() const {
  return 2 * ((max.x - min.x) + (max.y - min.y));
}

inline bool IntBounds2::Contains(int x, int y) const {
  return (unsigned)(x - min.x) <= (unsigned)(max.x - min.x) &&
         (unsigned)(y - min.y) <= (unsigned)(max.y - min.y);
}

inline bool Bounds2::Contains(float x, float y) const {
  return min.x <= x && x <= max.x && min.y <= y && y <= max.y;
}

inline void IntBounds2::Sort() {
  if (min.x > max.x)
    Swap(min.x, max.x);
  if (min.y > max.y)
    Swap(min.y, max.y);
}

inline void Bounds2::Sort() {
  if (min.x > max.x)
    Swap(min.x, max.x);
  if (min.y > max.y)
    Swap(min.y, max.y);
}

inline IntBounds2 IntBounds2::operator+(const IntVector2& offset) const {
  return IntBounds2(min + offset, max + offset);
}

inline IntBounds2 IntBounds2::operator-(const IntVector2& offset) const {
  return IntBounds2(min - offset, max - offset);
}

inline bool IntBounds2::Contains(const IntBounds2& other) const {
  return min.x <= other.min.x && other.max.x <= max.x
      && min.y <= other.min.y && other.max.y <= max.y;
}

inline bool Bounds2::Contains(const Bounds2& other) const {
  return min.x <= other.min.x && other.max.x <= max.x
      && min.y <= other.min.y && other.max.y <= max.y;
}

inline void IntBounds2::Inset(int dx, int dy) {
  auto delta = IntVector2(dx, dy);
  min += delta;
  max -= delta;
}

inline void Bounds2::Inset(float dx, float dy) {
  auto delta = Vector2(dx, dy);
  min += delta;
  max -= delta;
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_BOUNDS2_H_
