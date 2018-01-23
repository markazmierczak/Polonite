// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/CommonFactor.h"

#include "Base/Random/CryptoRandom.h"
#include "Base/Test/GTest.h"

namespace stp {

static inline uint32_t GcdRecursive32(uint32_t a, uint32_t b) {
  return b == 0 ? a : GcdRecursive32(b, a % b);
}

TEST(GcdTest, Gcd32) {
  struct Pair {
    uint32_t x;
    uint32_t y;
  };
  Pair input[] = {
    { 0, 0 },
    { 2, 0 },
    { 0, 1 },
    { 1, 1 },
    { 1, 2 },
    { ~UINT32_C(0), ~UINT32_C(1) },
  };

  for (Pair p : input)
    EXPECT_EQ(GcdRecursive32(p.x, p.y), GreatestCommonDivisor(p.x, p.y));

  for (int i = 0; i < 100; ++i) {
    uint32_t x = CryptoRandom::NextUInt32();
    uint32_t y = CryptoRandom::NextUInt32();
    EXPECT_EQ(GcdRecursive32(x, y), GreatestCommonDivisor(x, y));
  }
}

static inline uint64_t GcdRecursive64(uint64_t a, uint64_t b) {
  return b == 0 ? a : GcdRecursive64(b, a % b);
}

TEST(GcdTest, Gcd64) {
  struct Pair {
    uint64_t x;
    uint64_t y;
  };
  Pair input[] = {
    { 0, 0 },
    { 2, 0 },
    { 0, 1 },
    { 1, 1 },
    { 1, 2 },
    { ~UINT64_C(0), ~UINT64_C(1) },
  };

  for (Pair p : input)
    EXPECT_EQ(GcdRecursive64(p.x, p.y), GreatestCommonDivisor(p.x, p.y));

  for (int i = 0; i < 100; ++i) {
    uint32_t x = CryptoRandom::NextUInt64();
    uint32_t y = CryptoRandom::NextUInt64();
    EXPECT_EQ(GcdRecursive64(x, y), GreatestCommonDivisor(x, y));
  }
}

} // namespace stp
