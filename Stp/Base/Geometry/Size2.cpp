// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Geometry/Size2.h"

#include "Base/Io/TextWriter.h"

namespace stp {

void IntSize2::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out.WriteInteger(width);
  out.Write('x');
  out.WriteInteger(height);
}

IntSize2 Lerp(const IntSize2& a, const IntSize2& b, double t) {
  return IntSize2(
      Lerp(a.width, b.width, t),
      Lerp(a.height, b.height, t));
}

} // namespace stp
