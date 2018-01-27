// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_SIZE2_H_
#define STP_BASE_GEOMETRY_SIZE2_H_

#include "Geometry/Vector2.h"

namespace stp {

struct BASE_EXPORT IntSize2 {
  static constexpr IntSize2 Empty() { return IntSize2(); }

  constexpr IntSize2() : width(0), height(0) {}
  constexpr IntSize2(int width, int height)
      : width(width >= 0 ? width : 0), height(height >= 0 ? height : 0) {}

  bool IsEmpty() const { return width <= 0 || height <= 0; }

  static IntSize2 FromVector(const IntVector2& v) { return IntSize2(v.x, v.y); }
  IntVector2 ToVector() const { return IntVector2(width, height); }

  IntSize2 operator*(int scale) const { return IntSize2(width * scale, height * scale); }
  IntSize2 operator/(int scale) const { return IntSize2(width / scale, height / scale); }

  bool operator==(const IntSize2& o) const { return width == o.width && height == o.height; }
  bool operator!=(const IntSize2& o) const { return !operator==(o); }

  int width;
  int height;
};

inline IntSize2 Min(const IntSize2& lhs, const IntSize2& rhs) {
  return IntSize2(Min(lhs.width, rhs.width), Min(lhs.height, rhs.height));
}

inline IntSize2 Max(const IntSize2& lhs, const IntSize2& rhs) {
  return IntSize2(Max(lhs.width, rhs.width), Max(lhs.height, rhs.height));
}

BASE_EXPORT IntSize2 Lerp(const IntSize2& a, const IntSize2& b, double t);

} // namespace stp

#endif // STP_BASE_GEOMETRY_SIZE2_H_
