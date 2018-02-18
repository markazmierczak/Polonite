// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Thread/SpinLock.h"

#include "Base/Thread/NativeThread.h"

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#elif OS(POSIX)
#include <sched.h>
#endif

// The YIELD_PROCESSOR macro wraps an architecture specific-instruction that
// informs the processor we're in a busy wait, so it can handle the branch more
// intelligently and e.g. reduce power to our core or give more resources to the
// other hyper-thread on this core. See the following for context:
// https://software.intel.com/en-us/articles/benefitting-power-and-performance-sleep-loops
#if OS(WIN)
# define YIELD_PROCESSOR YieldProcessor()
#elif COMPILER(GCC) || COMPILER(CLANG)
# if CPU(X86_64) || CPU(X86_32)
#  define YIELD_PROCESSOR __asm__ __volatile__("pause")
# elif CPU(ARM32) || CPU(ARM64)
#  define YIELD_PROCESSOR __asm__ __volatile__("yield")
# endif
#endif

#ifndef YIELD_PROCESSOR
# warning "Processor yield not supported on this architecture."
# define YIELD_PROCESSOR ((void)0)
#endif

namespace stp {

void BasicSpinLock::AcquireSlow() {
  // The value of |YieldProcessorTries| is cargo culted from TCMalloc, Windows
  // critical section defaults, and various other recommendations.
  static const int YieldProcessorTries = 1000;

  do {

    do {

      for (int count = 0; count < YieldProcessorTries; ++count) {
        // Let the processor know we're spinning.
        YIELD_PROCESSOR;

        if (subtle::NoBarrier_Load(&lock_) == Free) {
          if (LIKELY(tryAcquire()))
            return;
        }
      }

      // Give the OS a chance to schedule something on this core.
      NativeThread::Yield();

    } while (subtle::NoBarrier_Load(&lock_) != Free);

  } while (UNLIKELY(!tryAcquire()));
}

} // namespace stp
