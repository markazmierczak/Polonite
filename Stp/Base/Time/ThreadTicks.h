// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_TIME_THREADTICKS_H_
#define STP_BASE_TIME_THREADTICKS_H_

#include "Base/Test/GTestProdUtil.h"
#include "Base/Time/TimeBase.h"

namespace stp {

// Represents a clock, specific to a particular thread, than runs only while the
// thread is running.
class BASE_EXPORT ThreadTicks : public TimeBase<ThreadTicks> {
 public:
  ThreadTicks() : TimeBase(0) {}

  // Returns true if ThreadTicks::Now() is supported on this system.
  static bool IsSupported() {
    #if (defined(_POSIX_THREAD_CPUTIME) && (_POSIX_THREAD_CPUTIME >= 0)) || OS(MAC) || OS(ANDROID)
    return true;
    #elif OS(WIN)
    return IsSupportedWin();
    #else
    return false;
    #endif
  }

  // Waits until the initialization is completed. Needs to be guarded with a
  // call to IsSupported().
  static void WaitUntilInitialized() {
    #if OS(WIN)
    WaitUntilInitializedWin();
    #endif
  }

  // Returns thread-specific CPU-time on systems that support this feature.
  // Needs to be guarded with a call to IsSupported(). Use this timer
  // to (approximately) measure how much time the calling thread spent doing
  // actual work vs. being de-scheduled. May return bogus results if the thread
  // migrates to another CPU between two calls. Returns an empty ThreadTicks
  // object until the initialization is completed. If a clock reading is
  // absolutely needed, call WaitUntilInitialized() before this method.
  static ThreadTicks Now();

  #if OS(WIN)
  // Similar to Now() above except this returns thread-specific CPU time for an
  // arbitrary thread. All comments for Now() method above apply apply to this
  // method as well.
  static ThreadTicks GetForThread(HANDLE thread_handle);
  #endif

  friend TextWriter& operator<<(TextWriter& out, const ThreadTicks& x) {
    FormatImpl(out, x); return out;
  }
  friend void Format(TextWriter& out, const ThreadTicks& x, const StringSpan& opts) {
    FormatImpl(out, x);
  }

 private:
  friend class TimeBase<ThreadTicks>;

  // Please use Now() or GetForThread() to create a new object. This is for
  // internal use and testing.
  explicit ThreadTicks(int64_t us) : TimeBase(us) {}

  #if OS(WIN)
  FRIEND_TEST_ALL_PREFIXES(ThreadTicks, TSCTicksPerSecond);

  // Returns the frequency of the TSC in ticks per second, or 0 if it hasn't
  // been measured yet. Needs to be guarded with a call to IsSupported().
  // This method is declared here rather than in the anonymous namespace to
  // allow testing.
  static double TSCTicksPerSecond();

  static bool IsSupportedWin();
  static void WaitUntilInitializedWin();
  #endif

  static void FormatImpl(TextWriter& out, ThreadTicks x);
};

} // namespace stp

#endif // STP_BASE_TIME_THREADTICKS_H_
