// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Random/Random.h"

#include "Base/Random/RandomInternal.h"
#include "Base/Type/Variable.h"

namespace stp {

inline int BasicRandom::Next() {
  return bit_cast<int32_t>(NextUInt32());
}

uint32_t BasicRandom::NextUInt32() {
  k_ = KMulFactor * (k_ & 0xFFFF) + (k_ >> 16);
  j_ = JMulFactor * (j_ & 0xFFFF) + (j_ >> 16);
  return ((k_ << 16) | (k_ >> 16)) + j_;
}

uint8_t BasicRandom::NextUInt8() {
  return NextUInt32() >> 24;
}

uint16_t BasicRandom::NextUInt16() {
  return NextUInt32() >> 16;
}

uint64_t BasicRandom::NextUInt64() {
  uint64_t hi = NextUInt32();
  return (hi << 32) | NextUInt32();
}

uint64_t BasicRandom::NextUInt64(uint64_t range) {
  ASSERT(range > 0);
  // We must discard random results above this number, as they would
  // make the random generator non-uniform (consider e.g. if
  // UINT64_MAX was 7 and |range| was 5, then a result of 1 would be twice
  // as likely as a result of 3 or 4).
  uint64_t max_acceptable_value = (UINT64_MAX / range) * range - 1;

  uint64_t value;
  do {
    value = NextUInt64();
  } while (value > max_acceptable_value);

  return value % range;
}

float BasicRandom::NextFloat() {
  return detail::RandomBitsToDouble(NextUInt32());
}

double BasicRandom::NextDouble() {
  return detail::RandomBitsToDouble(NextUInt64());
}

void BasicRandom::NextBytes(void* buffer, int length) {
  uint8_t* b = static_cast<uint8_t*>(buffer);

  for (; length > 4; length -= 4, b += 4) {
    uint32_t x = NextUInt32();
    for (int i = 0; i < 4; ++i, x >>= 8)
      b[i] = x & 0xFF;
  }
  for (; length; --length, ++b)
    *b = NextUInt8();
}

void BasicRandom::Seed(uint32_t seed) {
  // Initialize state variables with LCG.
  // We must ensure that both J and K are non-zero, otherwise the
  // multiply-with-carry step will forevermore return zero.
  k_ = NextLCG(seed);
  if (0 == k_) {
    k_ = NextLCG(k_);
  }
  j_ = NextLCG(k_);
  if (0 == j_) {
    j_ = NextLCG(j_);
  }
  ASSERT(0 != k_ && 0 != j_);
}

} // namespace stp
