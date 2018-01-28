// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Thread/WaitableEvent.h"

#include "Base/Debug/Log.h"
#include "Base/Math/SafeConversions.h"
#include "Base/Text/Format.h"
#include "Base/Time/TimeTicks.h"

#include <windows.h>

namespace stp {

WaitableEvent::WaitableEvent(ResetPolicy reset_policy, InitialState initial_state) {
  handle_.Reset(::CreateEvent(
      nullptr,
      reset_policy == ResetPolicy::Manual,
      initial_state == InitialState::Signaled,
      nullptr));

  // We're probably going to crash anyways if this is ever NULL, so we might as
  // well make our stack reports more informative by crashing here.
  ASSERT(handle_.IsValid());
}

WaitableEvent::WaitableEvent(win::ScopedHandle handle)
    : handle_(Move(handle)) {
  ASSERT(handle_.IsValid());
}

WaitableEvent::~WaitableEvent() = default;

void WaitableEvent::Reset() {
  ::ResetEvent(handle_.get());
}

void WaitableEvent::Signal() {
  ::SetEvent(handle_.get());
}

bool WaitableEvent::IsSignaled() {
  DWORD result = WaitForSingleObject(handle_.get(), 0);
  ASSERT(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT,
         "unexpected WaitForSingleObject result {}", result);
  return result == WAIT_OBJECT_0;
}

void WaitableEvent::Wait() {
  DWORD result = WaitForSingleObject(handle_.get(), INFINITE);
  // It is most unexpected that this should ever fail.  Help consumers learn
  // about it if it should ever fail.
  ASSERT_UNUSED(result == WAIT_OBJECT_0, result);
}

// Helper function called from TimedWait and TimedWaitUntil.
static bool WaitUntil(HANDLE handle, TimeTicks now, TimeTicks end_time) {
  TimeDelta delta = end_time - now;
  ASSERT(delta > TimeDelta());

  do {
    // On Windows, waiting for less than 1 ms results in WaitForSingleObject
    // returning promptly which may result in the caller code spinning.
    // We need to ensure that we specify at least the minimally possible 1 ms
    // delay unless the initial timeout was exactly zero.
    // FIXME make constexpr once MSVC 2017 bug is fixed
    TimeDelta MinDelta = TimeDelta::FromMilliseconds(1);
    if (delta < MinDelta)
      delta = MinDelta;
    
    // Truncate the timeout to milliseconds.
    DWORD timeout_ms = SaturatedCast<DWORD>(delta.InMilliseconds());
    DWORD result = ::WaitForSingleObject(handle, timeout_ms);
    switch (result) {
      case WAIT_OBJECT_0:
        return true;
      case WAIT_TIMEOUT:
        // TimedWait can time out earlier than the specified |timeout| on
        // Windows. To make this consistent with the posix implementation we
        // should guarantee that TimedWait doesn't return earlier than the
        // specified |max_time| and wait again for the remaining time.
        delta = end_time - TimeTicks::Now();
        break;
      default:
        ASSERT(false, "unexpected WaitForSingleObject result {}", result);
    }
  } while (delta > TimeDelta());
  return false;
}

bool WaitableEvent::TimedWait(TimeDelta wait_delta) {
  ASSERT(wait_delta >= TimeDelta());
  if (wait_delta.IsZero())
    return IsSignaled();

  TimeTicks now = TimeTicks::Now();
  // TimeTicks takes care of overflow including the cases when wait_delta
  // is a maximum value.
  return WaitUntil(handle_.get(), now, now + wait_delta);
}

bool WaitableEvent::TimedWaitUntil(TimeTicks end_time) {
  if (end_time.IsNull())
    return IsSignaled();

  TimeTicks now = TimeTicks::Now();
  if (end_time <= now)
    return IsSignaled();

  return WaitUntil(handle_.get(), now, end_time);
}

int WaitableEvent::WaitMany(WaitableEvent** events, int count) {
  HANDLE handles[MAXIMUM_WAIT_OBJECTS];
  ASSERT(count <= MAXIMUM_WAIT_OBJECTS,
         "can only wait on {} with WaitMany", MAXIMUM_WAIT_OBJECTS);

  for (int i = 0; i < count; ++i)
    handles[i] = events[i]->handle();

  // The cast is safe because count is small - see the CHECK above.
  DWORD result = WaitForMultipleObjects(
      static_cast<DWORD>(count),
      handles,
      FALSE,      // don't wait for all the objects
      INFINITE);  // no timeout

  if (result >= WAIT_OBJECT_0 + count) {
    LOG(ERROR, "WaitForMultipleObjects failed");
    return 0;
  }
  return result - WAIT_OBJECT_0;
}

} // namespace stp
