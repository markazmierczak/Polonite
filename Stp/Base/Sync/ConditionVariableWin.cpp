// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Sync/ConditionVariable.h"

#include "Base/Time/TimeDelta.h"

namespace stp {

ConditionVariable::ConditionVariable(BasicLock& user_lock)
    : srwlock_(&user_lock.native_object_)
    #if ASSERT_IS_ON()
    , user_lock_(&user_lock)
    #endif
{
  InitializeConditionVariable(&cv_);
}

ConditionVariable::~ConditionVariable() = default;

void ConditionVariable::Wait() {
  TimedWait(TimeDelta::FromMilliseconds(INFINITE));
}

void ConditionVariable::TimedWait(TimeDelta max_time) {
  DWORD timeout = static_cast<DWORD>(max_time.InMilliseconds());

  #if ASSERT_IS_ON()
  user_lock_->CheckHeldAndUnmark();
  #endif

  if (!SleepConditionVariableSRW(&cv_, srwlock_, timeout, 0)) {
    // On failure, we only expect the CV to timeout. Any other error value means
    // that we've unexpectedly woken up.
    // Note that WAIT_TIMEOUT != ERROR_TIMEOUT. WAIT_TIMEOUT is used with the
    // WaitFor* family of functions as a direct return value. ERROR_TIMEOUT is
    // used with GetLastError().
    ASSERT(GetLastError() == static_cast<DWORD>(ERROR_TIMEOUT));
  }

  #if ASSERT_IS_ON()
  user_lock_->CheckUnheldAndMark();
  #endif
}

void ConditionVariable::Broadcast() {
  WakeAllConditionVariable(&cv_);
}

void ConditionVariable::Signal() {
  WakeConditionVariable(&cv_);
}

} // namespace stp
