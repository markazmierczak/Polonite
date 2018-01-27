// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/PowerOfTwo.h"

#include "Base/Crypto/CryptoRandom.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(BitsTest, Log2Floor32) {
  auto slow = [](uint32_t x) {
    int l = 0;
    for (; x > 1; x /= 2)
      l++;
    return l;
  };

  const uint32_t input[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xAABBAABB),
    UINT32_C(0xFFFFFFFF),
  };

  for (uint32_t x : input)
    EXPECT_EQ(slow(x), Log2Floor(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.NextUInt32();
    if (!x)
      continue;
    EXPECT_EQ(slow(x), Log2Floor(x));
  }
}

TEST(BitsTest, Log2Floor64) {
  auto slow = [](uint64_t x) {
    int l = 0;
    for (; x > 1; x /= 2)
      l++;
    return l;
  };

  const uint64_t input[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0xF0F0F0F0F0F0F0F0),
    UINT64_C(0xFFF0F0FFFFF0F0FF),
    UINT64_C(0xFFFFFFFFFFFFFFFF),
  };

  for (uint64_t x : input)
    EXPECT_EQ(slow(x), Log2Floor(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.NextUInt64();
    if (!x)
      continue;
    EXPECT_EQ(slow(x), Log2Floor(x));
  }
}

TEST(BitsTest, Log2Ceil32) {
  auto slow = [](uint32_t x) {
    for (int i = 0; i < 32; ++i) {
      if ((UINT32_C(1) << i) >= x)
        return i;
    }
    return 32;
  };

  const uint32_t input[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xAABBAABB),
    UINT32_C(0xFFFFFFFF),
  };

  for (uint32_t x : input)
    EXPECT_EQ(slow(x), Log2Ceil(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.NextUInt32();
    if (!x)
      continue;
    EXPECT_EQ(slow(x), Log2Ceil(x));
  }
}

TEST(BitsTest, Log2Ceil64) {
  auto slow = [](uint64_t x) {
    for (int i = 0; i < 64; ++i) {
      if ((UINT64_C(1) << i) >= x)
        return i;
    }
    return 64;
  };

  const uint64_t input[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0xF0F0F0F0F0F0F0F0),
    UINT64_C(0xFFF0F0FFFFF0F0FF),
    UINT64_C(0xFFFFFFFFFFFFFFFF),
  };

  for (uint64_t x : input)
    EXPECT_EQ(slow(x), Log2Ceil(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.NextUInt64();
    if (!x)
      continue;
    EXPECT_EQ(slow(x), Log2Ceil(x));
  }
}

TEST(BitsTest, IsPowerOfTwo) {
  auto slow = [](uint32_t x) {
    bool flag = false;
    for (; x && !flag; x >>= 1) {
      if (x & 1)
        flag = true;
    }
    return flag && x == 0;
  };

  const uint32_t input[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xAABBAABB),
    UINT32_C(0xFFFFFFFF),
  };

  for (uint32_t x : input)
    EXPECT_EQ(slow(x), IsPowerOfTwo(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.NextUInt32();
    if (!x)
      continue;
    EXPECT_EQ(slow(x), IsPowerOfTwo(x));
  }
}

TEST(BitsTest, WhichPowerOfTwo) {
  for (int i = 0; i < 64; ++i)
    EXPECT_EQ(i, WhichPowerOfTwo(UINT64_C(1) << i));
}

TEST(BitsTest, RoundDownToPowerOfTwo32) {
  auto slow = [](uint32_t x) {
    uint32_t max = UINT32_C(1) << 31;
    if (x >= max)
      return max;

    uint32_t r = 1;
    while (r < x)
      r <<= 1;
    if (r != x)
      r >>= 1;
    return r;
  };

  const uint32_t input[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0x80000000),
    UINT32_C(0x80000001),
  };

  for (uint32_t x : input)
    EXPECT_EQ(slow(x), RoundDownToPowerOfTwo(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.NextUInt32();
    EXPECT_EQ(slow(x), RoundDownToPowerOfTwo(x));
  }
}

TEST(BitsTest, RoundDownToPowerOfTwo64) {
  auto slow = [](uint64_t x) {
    uint64_t max = UINT64_C(1) << 63;
    if (x >= max)
      return max;

    uint64_t r = 1;
    while (r < x)
      r <<= 1;
    if (r != x)
      r >>= 1;
    return r;
  };

  const uint64_t input[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0x8000000000000000),
    UINT64_C(0x8000000000000001),
  };

  for (uint64_t x : input)
    EXPECT_EQ(slow(x), RoundDownToPowerOfTwo(x)) << x;

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.NextUInt64();
    EXPECT_EQ(slow(x), RoundDownToPowerOfTwo(x));
  }
}

TEST(BitsTest, RoundUpToPowerOfTwo32) {
  auto slow = [](uint32_t x) {
    uint32_t r = 1;
    while (r < x)
      r <<= 1;
    return r;
  };

  const uint32_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0x80000000),
  };

  for (uint32_t x : input)
    EXPECT_EQ(slow(x), RoundUpToPowerOfTwo(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    // Shift one right to have last bit cleared.
    uint32_t x = rng.NextUInt32() >> 1;
    EXPECT_EQ(slow(x), RoundUpToPowerOfTwo(x));
  }
}

TEST(BitsTest, RoundUpToPowerOfTwo64) {
  auto slow = [](uint64_t x) {
    uint64_t r = 1;
    while (r < x)
      r <<= 1;
    return r;
  };

  const uint64_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0x8000000000000000),
  };

  for (uint64_t x : input)
    EXPECT_EQ(slow(x), RoundUpToPowerOfTwo(x)) << x;

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    // Shift one right to have last bit cleared.
    uint64_t x = rng.NextUInt64() >> 1;
    EXPECT_EQ(slow(x), RoundUpToPowerOfTwo(x));
  }
}

} // namespace stp
