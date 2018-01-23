// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Time/ThreadTicks.h"

#include "Base/Test/GTest.h"
#include "Base/Thread/Thread.h"
#include "Base/Time/TimeTicks.h"

namespace stp {

// Fails frequently on Android http://crbug.com/352633 with:
// Expected: (delta_thread.InMicroseconds()) > (0), actual: 0 vs 0
#if OS(ANDROID)
#define MAYBE_ThreadNow DISABLED_ThreadNow
#else
#define MAYBE_ThreadNow ThreadNow
#endif
TEST(ThreadTicks, MAYBE_ThreadNow) {
  if (ThreadTicks::IsSupported()) {
    ThreadTicks::WaitUntilInitialized();
    TimeTicks begin = TimeTicks::Now();
    ThreadTicks begin_thread = ThreadTicks::Now();
    // Make sure that ThreadNow value is non-zero.
    EXPECT_GT(begin_thread, ThreadTicks());
    // Sleep for 10 milliseconds to get the thread de-scheduled.
    ThisThread::SleepFor(TimeDelta::FromMilliseconds(10));
    ThreadTicks end_thread = ThreadTicks::Now();
    TimeTicks end = TimeTicks::Now();
    TimeDelta delta = end - begin;
    TimeDelta delta_thread = end_thread - begin_thread;
    // Make sure that some thread time have elapsed.
    EXPECT_GT(delta_thread.InMicroseconds(), 0);
    // But the thread time is at least 9ms less than clock time.
    TimeDelta difference = delta - delta_thread;
    EXPECT_GE(difference.InMicroseconds(), 9000);
  }
}

} // namespace stp
