// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_RANDOM_RANDOMINTERNAL_H_
#define STP_BASE_RANDOM_RANDOMINTERNAL_H_

#include "Base/Debug/Assert.h"
#include "Base/Math/Math.h"
#include "Base/Type/Limits.h"

namespace stp {
namespace detail {

// Given input |bits|, convert with maximum precision to a floating-point in
// the range [0, 1). Thread-safe.
template<typename T, typename U>
inline T RandomBitsToFloatingPoint(U bits) {
  // We try to get maximum precision by masking out as many bits as will fit
  // in the target type's mantissa, and raising it to an appropriate power to
  // produce output in the range [0, 1).  For IEEE 754 Ts, the mantissa
  // is expected to accommodate 53 bits.

  static const int Bits = Limits<T>::Digits;
  U random_bits = bits & ((U(1) << Bits) - 1);
  T result = LoadExponent(static_cast<T>(random_bits), -1 * Bits);
  ASSERT(0 <= result && result < 1);
  return result;
}

inline double RandomBitsToDouble(uint64_t bits) {
  return RandomBitsToFloatingPoint<double>(bits);
}
inline float RandomBitsToFloat(uint32_t bits) {
  return RandomBitsToFloatingPoint<float>(bits);
}

} // namespace detail
} // namespace stp

#endif // STP_BASE_RANDOM_RANDOMINTERNAL_H_
