// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/System/SysInfo.h"

#include "Base/System/Environment.h"
#include "Base/Test/GTest.h"
#include "Base/Test/PlatformTest.h"
#include "Base/Thread/Thread.h"

namespace stp {

typedef PlatformTest SysInfoTest;

TEST_F(SysInfoTest, AmountOfMem) {
  // We aren't actually testing that it's correct, just that it's sane.
  EXPECT_GT(SysInfo::AmountOfPhysicalMemory(), 0);
  EXPECT_GT(SysInfo::AmountOfPhysicalMemoryMB(), 0);
  // The maxmimal amount of virtual memory can be zero which means unlimited.
  EXPECT_GE(SysInfo::AmountOfVirtualMemory(), 0);
}

TEST_F(SysInfoTest, Uptime) {
  TimeDelta up_time_1 = SysInfo::Uptime();
  // UpTime() is implemented internally using TimeTicks::Now(), which documents
  // system resolution as being 1-15ms. Sleep a little longer than that.
  ThisThread::SleepFor(TimeDelta::FromMilliseconds(20));
  TimeDelta up_time_2 = SysInfo::Uptime();
  EXPECT_GT(up_time_1.InMicroseconds(), 0);
  EXPECT_GT(up_time_2.InMicroseconds(), up_time_1.InMicroseconds());
}

} // namespace stp
