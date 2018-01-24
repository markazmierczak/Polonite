// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ConditionVariable wraps pthreads condition variable synchronization or, on
// Windows, simulates it.  This functionality is very helpful for having
// several threads wait for an event, as is common with a thread pool managed
// by a master. The meaning of such an event in the (worker) thread pool
// scenario is that additional tasks are now available for processing.
//
// USAGE NOTE 1: spurious signal events are possible with this and
// most implementations of condition variables.  As a result, be
// *sure* to retest your condition before proceeding.  The following
// is a good example of doing this correctly:
//
// while (!work_to_be_done()) Wait(...);
//
// In contrast do NOT do the following:
//
// if (!work_to_be_done()) Wait(...);  // Don't do this.
//
// Especially avoid the above if you are relying on some other thread only
// issuing a signal up *if* there is work-to-do.  There can/will
// be spurious signals.  Recheck state on waiting thread before
// assuming the signal was intentional. Caveat caller ;-).
//
// USAGE NOTE 2: Broadcast() frees up all waiting threads at once,
// which leads to contention for the locks they all held when they
// called Wait().  This results in POOR performance.  A much better
// approach to getting a lot of threads out of Wait() is to have each
// thread (upon exiting Wait()) call Signal() to free up another
// Wait'ing thread.  Look at ConditionVariableTest.cpp for
// both examples.
//
// Broadcast() can be used nicely during teardown, as it gets the job
// done, and leaves no sleeping threads... and performance is less
// critical at that point.
//
// The semantics of Broadcast() are carefully crafted so that *all*
// threads that were waiting when the request was made will indeed
// get signaled.  Some implementations mess up, and don't signal them
// all, while others allow the wait to be effectively turned off (for
// a while while waiting threads come around).  This implementation
// appears correct, as it will not "lose" any signals, and will guarantee
// that all threads get signaled by Broadcast().
//
// This implementation offers support for "performance" in its selection of
// which thread to revive.  Performance, in direct contrast with "fairness,"
// assures that the thread that most recently began to Wait() is selected by
// Signal to revive.  Fairness would (if publicly supported) assure that the
// thread that has Wait()ed the longest is selected. The default policy
// may improve performance, as the selected thread may have a greater chance of
// having some of its stack data in various CPU caches.
//
// For a discussion of the many very subtle implementation details, see the FAQ
// at the end of ConditionVariableWin.cpp.

#ifndef STP_BASE_SYNC_CONDITIONVARIABLE_H_
#define STP_BASE_SYNC_CONDITIONVARIABLE_H_

#include "Base/Compiler/Os.h"
#include "Base/Export.h"
#include "Base/Sync/Lock.h"

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#elif OS(POSIX)
#include <pthread.h>
#endif

namespace stp {

class TimeDelta;

class BASE_EXPORT ConditionVariable {
 public:
  // Construct a cv for use with ONLY one user lock.
  explicit ConditionVariable(BasicLock* user_lock);

  ~ConditionVariable();

  // Wait() releases the caller's critical section atomically as it starts to
  // sleep, and the reacquires it when it is signaled. The wait functions are
  // susceptible to spurious wakeups. (See usage note 1 for more details.)
  void Wait();
  void TimedWait(TimeDelta max_time);

  // Broadcast() revives all waiting threads. (See usage note 2 for more
  // details.)
  void Broadcast();
  // Signal() revives one waiting thread.
  void Signal();

 private:

  #if OS(WIN)
  CONDITION_VARIABLE cv_;
  SRWLOCK* const srwlock_;
  #elif OS(POSIX)
  pthread_cond_t condition_;
  pthread_mutex_t* user_mutex_;
  #endif

  #if ASSERT_IS_ON()
  BasicLock* const user_lock_;  // Needed to adjust shadow lock state on wait.
  #endif

  DISALLOW_COPY_AND_ASSIGN(ConditionVariable);
};

} // namespace stp

#endif // STP_BASE_SYNC_CONDITIONVARIABLE_H_
