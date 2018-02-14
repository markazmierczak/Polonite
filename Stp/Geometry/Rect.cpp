// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Rect.h"

#include "Base/Debug/Assert.h"
#include "Base/Text/FormatMany.h"

namespace stp {

bool IntRect::Contains(int px, int py) const {
  return px >= left() && px < right() &&
         py >= top() && py < bottom();
}

bool IntRect::Contains(const IntRect& other) const {
  return left() <= other.left() && right() >= other.right() &&
         top() <= other.top() && bottom() >= other.bottom();
}

bool IntRect::Intersects(const IntRect& a, const IntRect& b) {
  return !a.IsEmpty() && !b.IsEmpty() &&
         a.left() < b.right() && b.left() < a.right() &&
         a.top() < b.bottom() && b.top() < a.bottom();
}

bool IntRect::TryIntersect(const IntRect& other) {
  IntRect intersection = Intersection(*this, other);
  if (intersection.IsEmpty())
    return false;
  *this = intersection;
  return true;
}

IntRect IntRect::Intersection(const IntRect& a, const IntRect& b) {
  int rx = Max(a.left(), b.left());
  int ry = Max(a.top(), b.top());
  int rr = Min(a.right(), b.right());
  int rb = Min(a.bottom(), b.bottom());

  // Return a clean empty rectangle for non-intersecting cases.
  if (rx >= rr || ry >= rb) {
    rx = 0;
    ry = 0;
    rr = 0;
    rb = 0;
  }

  return IntRect(rx, ry, rr - rx, rb - ry);
}

void IntRect::Unite(const IntRect& other) {
  if (other.IsEmpty())
    return;

  if (IsEmpty()) {
    *this = other;
    return;
  }

  int rx = Min(left(), other.left());
  int ry = Min(top(), other.top());
  int rr = Max(right(), other.right());
  int rb = Max(bottom(), other.bottom());

  position = IntPoint2(rx, ry);
  size = IntSize2(rr - rx, rb - ry);
}

IntRect IntRect::Union(const IntRect& a, const IntRect& b) {
  IntRect result = a;
  result.Unite(b);
  return result;
}

void IntRect::InsetAll(int amount) {
  Inset(amount, amount);
}

void IntRect::OutsetAll(int amount) {
  Outset(amount, amount);
}

void IntRect::Inset(int horizontal, int vertical) {
  Inset(IntRectExtents(horizontal, vertical, horizontal, vertical));
}

void IntRect::Outset(int horizontal, int vertical) {
  Outset(IntRectExtents(horizontal, vertical, horizontal, vertical));
}

void IntRect::Inset(const IntRectExtents& extents) {
  position += extents.lt;
  size = IntSize2(width() - extents.GetWidth(), height() - extents.GetHeight());
}

void IntRect::Outset(const IntRectExtents& extents) {
  position -= extents.lt;
  size = IntSize2(width() + extents.GetWidth(), height() + extents.GetHeight());
}

IntRect IntRect::Enclose(const IntPoint2& a, const IntPoint2& b) {
  int rx = Min(a.x, b.x);
  int ry = Min(a.y, b.y);
  int rr = Max(a.x, b.x);
  int rb = Max(a.y, b.y);
  return IntRect(rx, ry, rr - rx, rb - ry);
}

IntRect Lerp(const IntRect& a, const IntRect& b, double t) {
  return IntRect(
      Lerp(a.position, b.position, t),
      Lerp(a.size, b.size, t));
}

void IntRect::ToFormat(TextWriter& out, const StringSpan& opts) const {
  Format(out, position);
  out.Write(' ');
  Format(out, size);
}

IntRectExtents IntRectExtents::Compute(const IntRect& inner, const IntRect& outer) {
  return IntRectExtents(
      outer.left() - inner.left(),
      outer.top() - inner.top(),
      outer.right() - inner.right(),
      outer.bottom() - inner.bottom());
}

void IntRectExtents::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out.WriteInteger(left());
  out.Write(',');
  out.WriteInteger(top());
  out.Write(',');
  out.WriteInteger(right());
  out.Write(',');
  out.WriteInteger(bottom());
}

} // namespace stp
