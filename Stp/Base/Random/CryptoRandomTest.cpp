// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Random/CryptoRandom.h"

#include "Base/Containers/Sorting.h"
#include "Base/Test/GTest.h"

#include <limits.h>

namespace stp {

TEST(CryptoRandomTest, Int) {
  EXPECT_EQ(CryptoRandom::Next(0, 0), 0);
  EXPECT_EQ(CryptoRandom::Next(INT_MIN, INT_MIN), INT_MIN);
  EXPECT_EQ(CryptoRandom::Next(INT_MAX, INT_MAX), INT_MAX);

  // Check that the ASSERTs in NextInt() don't fire due to internal overflow.
  // There was a 50% chance of that happening, so calling it 40 times means
  // the chances of this passing by accident are tiny (9e-13).
  for (int i = 0; i < 40; ++i)
    CryptoRandom::Next(INT_MIN, INT_MAX);
}

TEST(CryptoRandomTest, Double) {
  // Force 64-bit precision, making sure we're not in a 80-bit FPU register.
  volatile double number = CryptoRandom::NextDouble();
  EXPECT_GT(1.0, number);
  EXPECT_LE(0.0, number);
}

TEST(CryptoRandomTest, Bytes) {
  const int BufferSize = 50;
  byte_t buffer[BufferSize];
  memset(buffer, 0, BufferSize);

  CryptoRandom::NextBytes(buffer, BufferSize);
  Sort(buffer, buffer + BufferSize);

  int unique_counter = 1;
  char prev = buffer[0];
  for (int i = 1; i < BufferSize; ++i) {
    if (prev != buffer[i]) {
      prev = buffer[i];
      unique_counter++;
    }
  }
  // Probability of occurrence of less than 25 unique bytes in 50 random bytes
  // is below 10^-25.
  EXPECT_LT(25, unique_counter);
}

TEST(CryptoRandomTest, IsUniform) {
  // Verify that generator has a uniform distribution.
  // This is a regression test that consistently failed when generator was
  // implemented this way:
  //
  //   return NextUInt64() % max;
  //
  // A degenerate case for such an implementation is e.g. a top of
  // range that is 2/3rds of the way to UINT64_MAX, in which case the
  // bottom half of the range would be twice as likely to occur as the
  // top half. A bit of calculus care of jar@ shows that the largest
  // measurable delta is when the top of the range is 3/4ths of the
  // way, so that's what we use in the test.
  const uint64_t TopOfRange = (UINT64_MAX / 4ull) * 3ull;
  const uint64_t ExpectedAverage = TopOfRange / 2ull;
  const uint64_t AllowedVariance = ExpectedAverage / 50ull;  // +/- 2%
  const int MinAttempts = 1000;
  const int MaxAttempts = 1000000;

  double cumulative_average = 0.0;
  int count = 0;
  while (count < MaxAttempts) {
    uint64_t value = CryptoRandom::NextUInt64(TopOfRange);
    cumulative_average = (count * cumulative_average + value) / (count + 1);

    // Don't quit too quickly for things to start converging, or we may have
    // a false positive.
    if (count > MinAttempts &&
        ExpectedAverage - AllowedVariance < cumulative_average &&
        cumulative_average < ExpectedAverage + AllowedVariance) {
      break;
    }

    ++count;
  }

  ASSERT_LT(count, MaxAttempts) << "Expected average was " <<
      ExpectedAverage << ", average ended at " << cumulative_average;
}

TEST(CryptoRandomTest, UInt64ProducesBothValuesOfAllBits) {
  // This tests to see that our underlying random generator is good
  // enough, for some value of good enough.
  uint64_t AllZeros = 0ull;
  uint64_t AllOnes = ~AllZeros;
  uint64_t found_ones = AllZeros;
  uint64_t found_zeros = AllOnes;

  for (int i = 0; i < 1000; ++i) {
    uint64_t value = CryptoRandom::NextUInt64();
    found_ones |= value;
    found_zeros &= value;

    if (found_zeros == AllZeros && found_ones == AllOnes)
      return;
  }

  FAIL() << "Didn't achieve all bit values in maximum number of tries.";
}

} // namespace stp
