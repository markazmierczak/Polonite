// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Time/TimeDelta.h"

#include "Base/Test/GTest.h"

#include <time.h>

namespace stp {

TEST(TimeDelta, FromAndIn) {
  EXPECT_EQ(13, TimeDelta::FromDays(13).InDays());
  EXPECT_EQ(13, TimeDelta::FromHours(13).InHours());
  EXPECT_EQ(13, TimeDelta::FromMinutes(13).InMinutes());
  EXPECT_EQ(13, TimeDelta::FromSeconds(13).InSeconds());
  EXPECT_EQ(13.0, TimeDelta::FromSeconds(13).InSecondsF());
  EXPECT_EQ(13, TimeDelta::FromMilliseconds(13).InMilliseconds());
  EXPECT_EQ(13.0, TimeDelta::FromMilliseconds(13).InMillisecondsF());
  EXPECT_EQ(13, TimeDelta::FromSecondsF(13.1).InSeconds());
  EXPECT_EQ(13.1, TimeDelta::FromSecondsF(13.1).InSecondsF());
  EXPECT_EQ(13, TimeDelta::FromMillisecondsF(13.3).InMilliseconds());
  EXPECT_EQ(13.3, TimeDelta::FromMillisecondsF(13.3).InMillisecondsF());
  EXPECT_EQ(13, TimeDelta::FromMicroseconds(13).InMicroseconds());
  EXPECT_EQ(3.456, TimeDelta::FromMillisecondsF(3.45678).InMillisecondsF());
}

#if OS(POSIX)
TEST(TimeDelta, TimeSpecConversion) {
  TimeDelta delta = TimeDelta::FromSeconds(0);
  struct timespec result = delta.toTimespec();
  EXPECT_EQ(result.tv_sec, 0);
  EXPECT_EQ(result.tv_nsec, 0);
  EXPECT_EQ(delta, TimeDelta::fromTimespec(result));

  delta = TimeDelta::FromSeconds(1);
  result = delta.toTimespec();
  EXPECT_EQ(result.tv_sec, 1);
  EXPECT_EQ(result.tv_nsec, 0);
  EXPECT_EQ(delta, TimeDelta::fromTimespec(result));

  delta = TimeDelta::FromMicroseconds(1);
  result = delta.toTimespec();
  EXPECT_EQ(result.tv_sec, 0);
  EXPECT_EQ(result.tv_nsec, 1000);
  EXPECT_EQ(delta, TimeDelta::fromTimespec(result));

  delta = TimeDelta::FromMicroseconds(TimeDelta::MicrosecondsPerSecond + 1);
  result = delta.toTimespec();
  EXPECT_EQ(result.tv_sec, 1);
  EXPECT_EQ(result.tv_nsec, 1000);
  EXPECT_EQ(delta, TimeDelta::fromTimespec(result));
}
#endif // OS(POSIX)

TEST(TimeDelta, GetMagnitude) {
  const int64_t zero = 0;
  EXPECT_EQ(TimeDelta::FromMicroseconds(zero),
            TimeDelta::FromMicroseconds(zero).GetMagnitude());

  const int64_t one = 1;
  const int64_t negative_one = -1;
  EXPECT_EQ(TimeDelta::FromMicroseconds(one),
            TimeDelta::FromMicroseconds(one).GetMagnitude());
  EXPECT_EQ(TimeDelta::FromMicroseconds(one),
            TimeDelta::FromMicroseconds(negative_one).GetMagnitude());

  const int64_t max_int64_minus_one = Limits<int64_t>::Max - 1;
  const int64_t min_int64_plus_two = Limits<int64_t>::Min + 2;
  EXPECT_EQ(TimeDelta::FromMicroseconds(max_int64_minus_one),
            TimeDelta::FromMicroseconds(max_int64_minus_one).GetMagnitude());
  EXPECT_EQ(TimeDelta::FromMicroseconds(max_int64_minus_one),
            TimeDelta::FromMicroseconds(min_int64_plus_two).GetMagnitude());
}

TEST(TimeDelta, NumericOperators) {
  int i = 2;
  EXPECT_EQ(TimeDelta::FromMilliseconds(2000),
            TimeDelta::FromMilliseconds(1000) * i);
  EXPECT_EQ(TimeDelta::FromMilliseconds(500),
            TimeDelta::FromMilliseconds(1000) / i);
  EXPECT_EQ(TimeDelta::FromMilliseconds(2000),
            TimeDelta::FromMilliseconds(1000) *= i);
  EXPECT_EQ(TimeDelta::FromMilliseconds(500),
            TimeDelta::FromMilliseconds(1000) /= i);
  EXPECT_EQ(TimeDelta::FromMilliseconds(2000),
            i * TimeDelta::FromMilliseconds(1000));

  EXPECT_EQ(TimeDelta::FromMilliseconds(2000),
            TimeDelta::FromMilliseconds(1000) * 2);
  EXPECT_EQ(TimeDelta::FromMilliseconds(500),
            TimeDelta::FromMilliseconds(1000) / 2);
  EXPECT_EQ(TimeDelta::FromMilliseconds(2000),
            TimeDelta::FromMilliseconds(1000) *= 2);
  EXPECT_EQ(TimeDelta::FromMilliseconds(500),
            TimeDelta::FromMilliseconds(1000) /= 2);
  EXPECT_EQ(TimeDelta::FromMilliseconds(2000),
            2 * TimeDelta::FromMilliseconds(1000));
}

} // namespace stp
