// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/CommonFactor.h"

#include "Base/Crypto/CryptoRandom.h"
#include "Base/Test/GTest.h"

namespace stp {

static inline uint32_t gcdRecursive32(uint32_t a, uint32_t b) {
  return b == 0 ? a : gcdRecursive32(b, a % b);
}

TEST(GcdTest, gcd32) {
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
    EXPECT_EQ(gcdRecursive32(p.x, p.y), greatestCommonDivisor(p.x, p.y));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.nextUint32();
    uint32_t y = rng.nextUint32();
    EXPECT_EQ(gcdRecursive32(x, y), greatestCommonDivisor(x, y));
  }
}

static inline uint64_t gcdRecursive64(uint64_t a, uint64_t b) {
  return b == 0 ? a : gcdRecursive64(b, a % b);
}

TEST(GcdTest, gcd64) {
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
    EXPECT_EQ(gcdRecursive64(p.x, p.y), greatestCommonDivisor(p.x, p.y));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.nextUint64();
    uint32_t y = rng.nextUint64();
    EXPECT_EQ(gcdRecursive64(x, y), greatestCommonDivisor(x, y));
  }
}

} // namespace stp
