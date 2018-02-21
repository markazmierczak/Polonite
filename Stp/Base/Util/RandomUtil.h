// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_RANDOMUTIL_H_
#define STP_BASE_UTIL_RANDOMUTIL_H_

#include "Base/Debug/Assert.h"
#include "Base/Math/Math.h"
#include "Base/Type/Limits.h"

namespace stp {

namespace detail {

// Given input |bits|, convert with maximum precision to a floating-point in
// the range [0, 1). Thread-safe.
template<typename T, typename U>
inline T RandomBitsToUnitFloatingPoint(U bits) {
  // We try to get maximum precision by masking out as many bits as will fit
  // in the target type's mantissa, and raising it to an appropriate power to
  // produce output in the range [0, 1).  For IEEE 754 Ts, the mantissa
  // is expected to accommodate 53 bits.

  constexpr int Bits = Limits<T>::Digits;
  U random_bits = bits & ((U(1) << Bits) - 1);
  T result = mathLoadExponent(static_cast<T>(random_bits), -1 * Bits);
  ASSERT(0 <= result && result < 1);
  return result;
}

} // namespace detail

class RandomUtil {
  STATIC_ONLY(RandomUtil);
 public:
  template<typename TGenerator>
  static float NextUnitFloat(TGenerator& generator) {
    return detail::RandomBitsToUnitFloatingPoint<float>(generator.NextUInt32());
  }
  template<typename TGenerator>
  static float NextUnitDouble(TGenerator& generator) {
    return detail::RandomBitsToUnitFloatingPoint<double>(generator.NextUInt64());
  }

  template<typename TGenerator>
  static uint64_t NextUInt64(TGenerator& generator, uint64_t range);

  template<typename TGenerator>
  static int NextInt(TGenerator& generator, int min, int max);
};

template<typename TGenerator>
uint64_t RandomUtil::NextUInt64(TGenerator& generator, uint64_t range) {
  ASSERT(range > 0);
  // We must discard random results above this number, as they would
  // make the random generator non-uniform (consider e.g. if
  // MAX_UINT64 was 7 and |range| was 5, then a result of 1 would be twice
  // as likely as a result of 3 or 4).
  uint64_t max_acceptable_value = (Limits<uint64_t>::Max / range) * range - 1;

  uint64_t value;
  do {
    value = generator.nextUint64();
  } while (value > max_acceptable_value);

  return value % range;
}

template<typename TGenerator>
int RandomUtil::NextInt(TGenerator& generator, int min, int max) {
  ASSERT(min <= max);

  int64_t range = static_cast<int64_t>(max) - min + 1;
  // |range| is at most UINT_MAX + 1, so the result of rand_uint64(range)
  // is at most UINT_MAX.  Hence it's safe to cast it from uint64_t to int64_t.
  int result = static_cast<int>(min + static_cast<int64_t>(NextUInt64(generator, range)));
  ASSERT(min <= result && result <= max);
  return result;
}

} // namespace stp

#endif // STP_BASE_UTIL_RANDOMUTIL_H_
