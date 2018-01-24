// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_LIMITS_H_
#define STP_BASE_GEOMETRY_LIMITS_H_

namespace stp {

template<typename T = float>
constexpr T NearlyZeroForGraphics = 1.f / (1 << 12);

} // namespace stp

#endif // STP_BASE_GEOMETRY_LIMITS_H_
