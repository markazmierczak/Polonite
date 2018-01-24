// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Sync/ConditionVariable.h"

#include "Base/Time/TimeDelta.h"

#include <errno.h>
#include <sys/time.h>

namespace stp {

ConditionVariable::ConditionVariable(BasicLock* user_lock)
    : user_mutex_(&user_lock->native_object_)
    #if ASSERT_IS_ON()
    , user_lock_(user_lock)
    #endif
{
  int rv;
  // Some platforms doesn't support monotonic clock based absolute deadlines.
  // On older Android platform versions, it's supported through the
  // non-standard pthread_cond_timedwait_monotonic_np. Newer platform
  // versions have pthread_condattr_setclock (since Android 5.0).
  // Mac can use relative time deadlines.
  #if !OS(DARWIN) && !(OS(ANDROID) && defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC))
  pthread_condattr_t attrs;
  rv = pthread_condattr_init(&attrs);
  ASSERT(rv == 0);
  pthread_condattr_setclock(&attrs, CLOCK_MONOTONIC);
  rv = pthread_cond_init(&condition_, &attrs);
  pthread_condattr_destroy(&attrs);
  #else
  rv = pthread_cond_init(&condition_, NULL);
  #endif
  ASSERT_UNUSED(rv == 0, rv);
}

ConditionVariable::~ConditionVariable() {
  #if OS(DARWIN)
  // This hack is necessary to avoid a fatal pthreads subsystem bug in the
  // Darwin kernel.
  {
    Lock lock;
    AutoLock l(&lock);
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1;
    pthread_cond_timedwait_relative_np(&condition_, lock.lock_.native_handle(), &ts);
  }
  #endif

  int rv = pthread_cond_destroy(&condition_);
  ASSERT_UNUSED(rv == 0, rv);
}

void ConditionVariable::Wait() {
  #if ASSERT_IS_ON()
  user_lock_->CheckHeldAndUnmark();
  #endif
  int rv = pthread_cond_wait(&condition_, user_mutex_);
  ASSERT_UNUSED(rv == 0, rv);
  #if ASSERT_IS_ON()
  user_lock_->CheckUnheldAndMark();
  #endif
}

void ConditionVariable::TimedWait(TimeDelta max_time) {
  struct timespec relative_time = max_time.ToTimeSpec();

  #if ASSERT_IS_ON()
  user_lock_->CheckHeldAndUnmark();
  #endif

  #if OS(DARWIN)
  int rv = pthread_cond_timedwait_relative_np(&condition_, user_mutex_, &relative_time);
  #else
  // The timeout argument to pthread_cond_timedwait is in absolute time.
  struct timespec absolute_time;
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  absolute_time.tv_sec = now.tv_sec;
  absolute_time.tv_nsec = now.tv_nsec;

  absolute_time.tv_sec += relative_time.tv_sec;
  absolute_time.tv_nsec += relative_time.tv_nsec;
  absolute_time.tv_sec += absolute_time.tv_nsec / TimeDelta::NanosecondsPerSecond;
  absolute_time.tv_nsec %= TimeDelta::NanosecondsPerSecond;
  ASSERT(absolute_time.tv_sec >= now.tv_sec);  // Overflow paranoia

  #if OS(ANDROID) && defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC)
  int rv = pthread_cond_timedwait_monotonic_np(&condition_, user_mutex_, &absolute_time);
  #else
  int rv = pthread_cond_timedwait(&condition_, user_mutex_, &absolute_time);
  #endif

  #endif // !OS(DARWIN)

  // On failure, we only expect the CV to timeout. Any other error value means
  // that we've unexpectedly woken up.
  ASSERT_UNUSED(rv == 0 || rv == ETIMEDOUT, rv);
  #if ASSERT_IS_ON()
  user_lock_->CheckUnheldAndMark();
  #endif
}

void ConditionVariable::Broadcast() {
  int rv = pthread_cond_broadcast(&condition_);
  ASSERT_UNUSED(rv == 0, rv);
}

void ConditionVariable::Signal() {
  int rv = pthread_cond_signal(&condition_);
  ASSERT_UNUSED(rv == 0, rv);
}

} // namespace stp
