// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Time/TimeTicks.h"

#include "Base/Test/GTest.h"
#include "Base/Thread/Thread.h"
#include "Base/Type/FormattableToString.h"

namespace stp {

TEST(TimeTicks, Deltas) {
  for (int index = 0; index < 50; index++) {
    TimeTicks ticks_start = TimeTicks::Now();
    ThisThread::SleepFor(TimeDelta::FromMilliseconds(10));
    TimeTicks ticks_stop = TimeTicks::Now();
    TimeDelta delta = ticks_stop - ticks_start;
    // Note:  Although we asked for a 10ms sleep, if the
    // time clock has a finer granularity than the Sleep()
    // clock, it is quite possible to wakeup early.  Here
    // is how that works:
    //      Time(ms timer)      Time(us timer)
    //          5                   5010
    //          6                   6010
    //          7                   7010
    //          8                   8010
    //          9                   9000
    // Elapsed  4ms                 3990us
    //
    // Unfortunately, our InMilliseconds() function truncates
    // rather than rounds.  We should consider fixing this
    // so that our averages come out better.
    EXPECT_GE(delta.InMilliseconds(), 9);
    EXPECT_GE(delta.InMicroseconds(), 9000);
    EXPECT_EQ(delta.InSeconds(), 0);
  }
}

static void HighResClockTest(TimeTicks (*GetTicks)()) {
  // Why do we loop here?
  // We're trying to measure that intervals increment in a VERY small amount
  // of time --  less than 15ms.  Unfortunately, if we happen to have a
  // context switch in the middle of our test, the context switch could easily
  // exceed our limit.  So, we iterate on this several times.  As long as we're
  // able to detect the fine-granularity timers at least once, then the test
  // has succeeded.

  const int kTargetGranularityUs = 15000;  // 15ms

  bool success = false;
  int retries = 100;  // Arbitrary.
  TimeDelta delta;
  while (!success && retries--) {
    TimeTicks ticks_start = GetTicks();
    // Loop until we can detect that the clock has changed.  Non-HighRes timers
    // will increment in chunks, e.g. 15ms.  By spinning until we see a clock
    // change, we detect the minimum time between measurements.
    do {
      delta = GetTicks() - ticks_start;
    } while (delta.InMilliseconds() == 0);

    if (delta.InMicroseconds() <= kTargetGranularityUs)
      success = true;
  }

  // In high resolution mode, we expect to see the clock increment
  // in intervals less than 15ms.
  EXPECT_TRUE(success);
}

TEST(TimeTicks, HighRes) {
  HighResClockTest(&TimeTicks::Now);
}

TEST(TimeTicks, SnappedToNextTickBasic) {
  TimeTicks phase = TimeTicks::FromInternalValue(4000);
  TimeDelta interval = TimeDelta::FromMicroseconds(1000);
  TimeTicks timestamp;

  // Timestamp in previous interval.
  timestamp = TimeTicks::FromInternalValue(3500);
  EXPECT_EQ(4000, timestamp.SnappedToNextTick(phase, interval).ToInternalValue());

  // Timestamp in next interval.
  timestamp = TimeTicks::FromInternalValue(4500);
  EXPECT_EQ(5000, timestamp.SnappedToNextTick(phase, interval).ToInternalValue());

  // Timestamp multiple intervals before.
  timestamp = TimeTicks::FromInternalValue(2500);
  EXPECT_EQ(3000, timestamp.SnappedToNextTick(phase, interval).ToInternalValue());

  // Timestamp multiple intervals after.
  timestamp = TimeTicks::FromInternalValue(6500);
  EXPECT_EQ(7000, timestamp.SnappedToNextTick(phase, interval).ToInternalValue());

  // Timestamp on previous interval.
  timestamp = TimeTicks::FromInternalValue(3000);
  EXPECT_EQ(3000, timestamp.SnappedToNextTick(phase, interval).ToInternalValue());

  // Timestamp on next interval.
  timestamp = TimeTicks::FromInternalValue(5000);
  EXPECT_EQ(5000, timestamp.SnappedToNextTick(phase, interval).ToInternalValue());

  // Timestamp equal to phase.
  timestamp = TimeTicks::FromInternalValue(4000);
  EXPECT_EQ(4000, timestamp.SnappedToNextTick(phase, interval).ToInternalValue());
}

TEST(TimeTicks, SnappedToNextTickOverflow) {
  // int(big_timestamp / interval) < 0, so this causes a crash if the number of
  // intervals elapsed is attempted to be stored in an int.
  TimeTicks phase = TimeTicks::FromInternalValue(0);
  TimeDelta interval = TimeDelta::FromMicroseconds(4000);
  TimeTicks big_timestamp =
      TimeTicks::FromInternalValue(8635916564000);

  EXPECT_EQ(8635916564000,
            big_timestamp.SnappedToNextTick(phase, interval).ToInternalValue());
  EXPECT_EQ(8635916564000,
            big_timestamp.SnappedToNextTick(big_timestamp, interval).ToInternalValue());
}

TEST(TimeTicksLogging, ZeroTime) {
  TimeTicks zero;
  EXPECT_EQ("0 bogo-microseconds", formattableToString(zero));
}

TEST(TimeTicksLogging, FortyYearsLater) {
  TimeTicks forty_years_later = TimeTicks() + TimeDelta::FromDays(365.25 * 40);
  EXPECT_EQ("1262304000000000 bogo-microseconds", formattableToString(forty_years_later));
}

} // namespace stp
