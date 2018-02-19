// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Crypto/CryptoRandom.h"

#include "Base/Containers/Array.h"
#include "Base/Containers/Sorting.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(CryptoRandomTest, Bytes) {
  Array<byte_t, 50> buffer;

  CryptoRandom().generate(makeBufferSpan(buffer));
  sortSpan<byte_t>(buffer);

  int unique_counter = 1;
  char prev = buffer[0];
  for (int i = 1; i < buffer.size(); ++i) {
    if (prev != buffer[i]) {
      prev = buffer[i];
      unique_counter++;
    }
  }
  // Probability of occurrence of less than 25 unique bytes in 50 random bytes
  // is below 10^-25.
  EXPECT_LT(25, unique_counter);
}

TEST(CryptoRandomTest, UInt64ProducesBothValuesOfAllBits) {
  // This tests to see that our underlying random generator is good
  // enough, for some value of good enough.
  uint64_t AllZeros = 0ull;
  uint64_t AllOnes = ~AllZeros;
  uint64_t found_ones = AllZeros;
  uint64_t found_zeros = AllOnes;

  CryptoRandom crypto_random;
  for (int i = 0; i < 1000; ++i) {
    uint64_t value = crypto_random.nextUint64();
    found_ones |= value;
    found_zeros &= value;

    if (found_zeros == AllZeros && found_ones == AllOnes)
      return;
  }

  FAIL() << "Didn't achieve all bit values in maximum number of tries.";
}

} // namespace stp
