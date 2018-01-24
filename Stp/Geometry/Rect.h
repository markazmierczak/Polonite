// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_RECT_H_
#define STP_BASE_GEOMETRY_RECT_H_

#include "Geometry/Size2.h"
#include "Geometry/Vector2.h"

namespace stp {

struct IntRectExtents;

// Rect is valid only in coordinate system
// where X axis is growing right and Y growing down.
struct BASE_EXPORT IntRect {
  IntRect() {}
  IntRect(const IntPoint2& position, const IntSize2& size) : position(position), size(size) {}
  IntRect(int x, int y, int width, int height) : position(x, y), size(width, height) {}

  IntRect(int width, int height) : size(width, height) {}
  explicit IntRect(const IntSize2& size) : size(size) {}

  int width() const { return size.width; }
  int height() const { return size.height; }

  int left() const { return position.x; }
  int top() const { return position.y; }
  int right() const { return position.x + width(); }
  int bottom() const { return position.y + height(); }

  IntPoint2 GetTopLeft() const { return IntPoint2(left(), top()); }
  IntPoint2 GetTopRight() const { return IntPoint2(right(), top()); }
  IntPoint2 GetBottomLeft() const { return IntPoint2(left(), bottom()); }
  IntPoint2 GetBottomRight() const { return IntPoint2(right(), bottom()); }

  bool IsEmpty() const { return size.IsEmpty(); }

  IntPoint2 GetCenterPoint() const { return position + size.ToVector() / 2; }

  // Returns true if the point identified by point_x and point_y falls inside
  // this rectangle.  The point (x, y) is inside the rectangle, but the
  // point (x + width, y + height) is not.
  bool Contains(int point_x, int point_y) const;
  bool Contains(IntPoint2 point) const { return Contains(point.x, point.y); }

  // If either rect is empty returns false.
  bool Contains(const IntRect& rect) const;

  // An empty rectangle doesn't intersect any rectangle.
  static bool Intersects(const IntRect& a, const IntRect& b) WARN_UNUSED_RESULT;

  static IntRect Intersection(const IntRect& a, const IntRect& b) WARN_UNUSED_RESULT;
  static IntRect Union(const IntRect& a, const IntRect& b) WARN_UNUSED_RESULT;

  bool TryIntersect(const IntRect& rect);
  void Unite(const IntRect& rect);

  void InsetAll(int amount);
  void Inset(int horizontal, int vertical);
  void Inset(const IntRectExtents& extents);

  void OutsetAll(int amount);
  void Outset(int horizontal, int vertical);
  void Outset(const IntRectExtents& extents);

  static IntRect Enclose(const IntPoint2& a, const IntPoint2& b);

  void operator+=(const IntVector2& offset) { position += offset; }
  void operator-=(const IntVector2& offset) { position -= offset; }

  IntRect operator+(const IntVector2& delta) const { return IntRect(position + delta, size); }
  IntRect operator-(const IntVector2& delta) const { return IntRect(position - delta, size); }

  IntRect operator*(int factor) const { return IntRect(position * factor, size * factor); }
  IntRect operator/(int factor) const { return IntRect(position / factor, size / factor); }

  bool operator==(const IntRect& o) const { return position == o.position && size == o.size; }
  bool operator!=(const IntRect& other) const { return !operator==(other); }

  void ToFormat(TextWriter& out, const StringSpan& opts) const;

  IntPoint2 position;
  IntSize2 size;
};

struct BASE_EXPORT IntRectExtents {
  static IntRectExtents Compute(const IntRect& inner, const IntRect& outer);

  IntRectExtents() {}
  IntRectExtents(const IntVector2& lt, const IntVector2& rb) : lt(lt), rb(rb) {}

  IntRectExtents(int horizontal, int vertical)
      : lt(horizontal, vertical), rb(horizontal, vertical) {}

  IntRectExtents(int left, int top, int right, int bottom)
      : lt(left, top), rb(right, bottom) {}

  bool IsZero() const { return lt.IsZero() && rb.IsZero(); }

  IntRectExtents operator-() const { return IntRectExtents(-lt, -rb); }

  void operator+=(const IntRectExtents& other) { lt += other.lt; rb += other.rb; }
  void operator-=(const IntRectExtents& other) { lt -= other.lt; rb -= other.rb; }

  IntRectExtents operator+(const IntRectExtents& o) const;
  IntRectExtents operator-(const IntRectExtents& o) const;

  int left() const { return lt.x; }
  int top() const { return lt.y; }
  int right() const { return rb.x; }
  int bottom() const { return rb.y; }

  int GetWidth() const { return lt.x + rb.x; }
  int GetHeight() const { return lt.y + rb.y; }

  void ToFormat(TextWriter& out, const StringSpan& opts) const;

  IntVector2 lt;
  IntVector2 rb;
};

BASE_EXPORT IntRect Lerp(const IntRect& a, const IntRect& b, double t);

inline IntRectExtents IntRectExtents::operator+(const IntRectExtents& o) const {
  return IntRectExtents(lt + o.lt, rb + o.rb);
}

inline IntRectExtents IntRectExtents::operator-(const IntRectExtents& o) const {
  return IntRectExtents(lt - o.lt, rb - o.rb);
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_RECT_H_
