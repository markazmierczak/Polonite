// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_TEST_PLATFORMTEST_H_
#define STP_BASE_TEST_PLATFORMTEST_H_

#include "Base/Test/GTest.h"

#if defined(GTEST_OS_MAC)
#ifdef __OBJC__
@class NSAutoreleasePool;
#else
class NSAutoreleasePool;
#endif

// The purpose of this class is to provide a hook for platform-specific
// operations across unit tests. For example, on the Mac, it creates and
// releases an outer NSAutoreleasePool for each test case.  For now, it's only
// implemented on the Mac. To enable this for another platform, just adjust
// the #ifdefs and add a PlatformTest<platform>.cpp implementation file.
class PlatformTest : public testing::Test {
 public:
  virtual ~PlatformTest();

 protected:
  PlatformTest();

 private:
  NSAutoreleasePool* pool_;
};
#else
typedef testing::Test PlatformTest;
#endif // GTEST_OS_MAC

#endif // STP_BASE_TEST_PLATFORMTEST_H_
