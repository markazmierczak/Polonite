// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/RandomUtil.h"

#include "Base/Crypto/CryptoRandom.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(RandomUtilTest, IsUniform) {
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
  CryptoRandom crypto_random;
  for (; count < MaxAttempts; ++count) {
    uint64_t value = RandomUtil::NextUInt64(crypto_random, TopOfRange);
    cumulative_average = (count * cumulative_average + value) / (count + 1);

    // Don't quit too quickly for things to start converging, or we may have
    // a false positive.
    if (count > MinAttempts &&
        ExpectedAverage - AllowedVariance < cumulative_average &&
        cumulative_average < ExpectedAverage + AllowedVariance) {
      break;
    }
  }

  ASSERT_LT(count, MaxAttempts) << "Expected average was " <<
      ExpectedAverage << ", average ended at " << cumulative_average;
}

} // namespace stp
