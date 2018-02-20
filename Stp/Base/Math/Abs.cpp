// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/Abs.h"

#include "Base/Math/RawFloat.h"

namespace stp {

bool isNearUlp(float x, float y) {
  return isNearUlp(RawFloat(x), RawFloat(y));
}

bool isNearUlp(double x, double y) {
  return isNearUlp(RawDouble(x), RawDouble(y));
}

} // namespace stp
