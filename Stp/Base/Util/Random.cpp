// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/Random.h"

namespace stp {

uint32_t BasicRandom::NextUInt32() {
  k_ = KMulFactor * (k_ & 0xFFFF) + (k_ >> 16);
  j_ = JMulFactor * (j_ & 0xFFFF) + (j_ >> 16);
  return ((k_ << 16) | (k_ >> 16)) + j_;
}

uint64_t BasicRandom::NextUInt64() {
  uint64_t hi = NextUInt32();
  return (hi << 32) | NextUInt32();
}

void BasicRandom::Fill(MutableBufferSpan buffer) {
  auto* out = static_cast<uint8_t*>(buffer.data());
  int length = buffer.size();

  while (length > 0) {
    uint32_t word = NextUInt32();
    for (int i = 0; i < 4 && length > 0; ++i) {
      out[i] = static_cast<uint8_t>(word & 0xFF);
      word >>= 8;
      --length;
    }
  }
}

void BasicRandom::Seed(uint32_t seed) {
  // Initialize state variables with LCG.
  // We must ensure that both J and K are non-zero, otherwise the
  // multiply-with-carry step will forever return zero.
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
