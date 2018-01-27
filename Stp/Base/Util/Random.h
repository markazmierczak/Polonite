// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_RANDOM_H_
#define STP_BASE_UTIL_RANDOM_H_

#include "Base/Containers/BufferSpan.h"

namespace stp {

#define BASIC_RANDOM_INITIALIZER { 0, 0 }

// Independent pseudo random generator, optimized to be fast.
// This is NOT cryptographically secure random number generator, nor is thread-safe.
class BASE_EXPORT BasicRandom {
 public:
  void Fill(MutableBufferSpan buffer);

  // Returns a pseudo-random number in range [0, UINT32_MAX].
  uint32_t NextUInt32();

  // Returns a pseudo-random number in range [0, UINT64_MAX].
  uint64_t NextUInt64();

  // Reset the random object.
  void Seed(uint32_t seed);

  uint32_t k_;
  uint32_t j_;

 private:
  static uint32_t NextLCG(uint32_t seed) { return MulFactor * seed + AddFactor; }

  // See "Numerical Recipes in C", 1992 page 284 for these constants
  // For the LCG that sets the initial state from a seed
  static constexpr uint32_t MulFactor = 1664525;
  static constexpr uint32_t AddFactor = 1013904223;

  // Constants for the multiply-with-carry steps
  static constexpr uint32_t KMulFactor = 30345;
  static constexpr uint32_t JMulFactor = 18000;
};

class Random : public BasicRandom {
 public:
  explicit Random(uint32_t initial_seed = 0) {
    Seed(initial_seed);
  }

 private:
  using BasicRandom::k_;
  using BasicRandom::j_;
};

} // namespace stp

#endif // STP_BASE_UTIL_RANDOM_H_
