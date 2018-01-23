// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/Abs.h"

#include "Base/Math/RawFloat.h"

namespace stp {

bool IsNearUlp(float x, float y) {
  return IsNearUlp(RawFloat(x), RawFloat(y));
}

bool IsNearUlp(double x, double y) {
  return IsNearUlp(RawDouble(x), RawDouble(y));
}

} // namespace stp
