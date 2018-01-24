// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Sync/WaitableEvent.h"

#include "Base/Sync/ConditionVariable.h"
#include "Base/Sync/Lock.h"
#include "Base/Time/TimeTicks.h"

// A WaitableEvent on POSIX is implemented as a wait-list. Currently we don't
// support cross-process events (where one process can signal an event which
// others are waiting on). Because of this, we can avoid having one thread per
// listener in several cases.
//
// The WaitableEvent maintains a list of waiters, protected by a lock. Each
// waiter is either an async wait, in which case we have a Task and the
// MessageLoop to run it on, or a blocking wait, in which case we have the
// condition variable to signal.
//
// Waiting involves grabbing the lock and adding oneself to the wait list. Async
// waits can be canceled, which means grabbing the lock and removing oneself
// from the list.
//
// Waiting on multiple events is handled by adding a single, synchronous wait to
// the wait-list of many events. An event passes a pointer to itself when
// firing a waiter and so we can store that pointer to find out which event
// triggered.

namespace stp {

// This is just an abstract base class for waking the two types of waiters
WaitableEvent::WaitableEvent(ResetPolicy reset_policy, InitialState initial_state)
    : kernel_(WaitableEventKernel::Create(reset_policy, initial_state)) {}

WaitableEvent::~WaitableEvent() = default;

void WaitableEvent::Reset() {
  AutoLock locked(&kernel_->lock_);
  kernel_->signaled_ = false;
}

void WaitableEvent::Signal() {
  AutoLock locked(&kernel_->lock_);

  if (kernel_->signaled_)
    return;

  if (kernel_->manual_reset_) {
    SignalAll();
    kernel_->signaled_ = true;
  } else {
    // In the case of auto reset, if no waiters were woken, we remain
    // signaled.
    if (!SignalOne())
      kernel_->signaled_ = true;
  }
}

bool WaitableEvent::IsSignaled() {
  AutoLock locked(&kernel_->lock_);

  const bool result = kernel_->signaled_;
  if (result && !kernel_->manual_reset_)
    kernel_->signaled_ = false;
  return result;
}

// Synchronous waits

// This is a synchronous waiter. The thread is waiting on the given condition
// variable and the fired flag in this object.
class SyncWaiter : public WaitableEvent::Waiter {
 public:
  SyncWaiter()
      : fired_(false),
        signaling_event_(nullptr),
        lock_(),
        cv_(&lock_) {
  }

  bool Fire(WaitableEvent* signaling_event) override {
    AutoLock locked(&lock_);

    if (fired_)
      return false;

    fired_ = true;
    signaling_event_ = signaling_event;

    cv_.Broadcast();

    // Unlike AsyncWaiter objects, SyncWaiter objects are stack-allocated on
    // the blocking thread's stack.  There is no |delete this;| in Fire.  The
    // SyncWaiter object is destroyed when it goes out of scope.

    return true;
  }

  ALWAYS_INLINE WaitableEvent* signaling_event() const { return signaling_event_; }

  // These waiters are always stack allocated and don't delete themselves. Thus
  // there's no problem and the ABA tag is the same as the object pointer.
  bool Compare(void* tag) override { return this == tag; }

  // Called with lock held.
  ALWAYS_INLINE bool fired() const { return fired_; }

  // During a TimedWait, we need a way to make sure that an auto-reset
  // WaitableEvent doesn't think that this event has been signaled between
  // unlocking it and removing it from the wait-list. Called with lock held.
  void Disable() { fired_ = true; }

  ALWAYS_INLINE Lock* lock() { return &lock_; }

  ALWAYS_INLINE ConditionVariable* cv() { return &cv_; }

 private:
  bool fired_;
  WaitableEvent* signaling_event_;  // The WaitableEvent which woke us
  Lock lock_;
  ConditionVariable cv_;
};

void WaitableEvent::Wait() {
  bool result = TimedWaitUntilInternal(nullptr);
  ASSERT(result, "TimedWait() should never fail with infinite timeout");
  ALLOW_UNUSED_LOCAL(result);
}

bool WaitableEvent::TimedWait(TimeDelta wait_delta) {
  TimeTicks end_time = TimeTicks::Now() + wait_delta;
  // TimeTicks takes care of overflow including the cases when wait_delta
  // is a maximum value.
  return TimedWaitUntilInternal(&end_time);
}

bool WaitableEvent::TimedWaitUntil(TimeTicks end_time) {
  return TimedWaitUntilInternal(&end_time);
}

bool WaitableEvent::TimedWaitUntilInternal(const TimeTicks* end_time) {
  const bool finite_time = end_time != nullptr;

  kernel_->lock_.Acquire();
  if (kernel_->signaled_) {
    if (!kernel_->manual_reset_) {
      // In this case we were signaled when we had no waiters. Now that
      // someone has waited upon us, we can automatically reset.
      kernel_->signaled_ = false;
    }

    kernel_->lock_.Release();
    return true;
  }

  SyncWaiter sw;
  sw.lock()->Acquire();

  Enqueue(&sw);
  kernel_->lock_.Release();
  // We are violating locking order here by holding the SyncWaiter lock but not
  // the WaitableEvent lock. However, this is safe because we don't lock @lock_
  // again before unlocking it.

  for (;;) {
    const TimeTicks current_time = TimeTicks::Now();

    if (sw.fired() || (finite_time && current_time >= *end_time)) {
      const bool return_value = sw.fired();

      // We can't acquire @lock_ before releasing the SyncWaiter lock (because
      // of locking order), however, in between the two a signal could be fired
      // and @sw would accept it, however we will still return false, so the
      // signal would be lost on an auto-reset WaitableEvent. Thus we call
      // Disable which makes sw::Fire return false.
      sw.Disable();
      sw.lock()->Release();

      // This is a bug that has been enshrined in the interface of
      // WaitableEvent now: |Dequeue| is called even when |sw.fired()| is true,
      // even though it'll always return false in that case. However, taking
      // the lock ensures that |Signal| has completed before we return and
      // means that a WaitableEvent can synchronise its own destruction.
      kernel_->lock_.Acquire();
      kernel_->Dequeue(&sw, &sw);
      kernel_->lock_.Release();

      return return_value;
    }

    if (finite_time) {
      const TimeDelta max_wait(*end_time - current_time);
      sw.cv()->TimedWait(max_wait);
    } else {
      sw.cv()->Wait();
    }
  }
}

int WaitableEvent::WaitMany(WaitableEvent** raw_waitables, int count) {
  ASSERT(count, "cannot wait on no events");

  // We need to acquire the locks in a globally consistent order. Thus we sort
  // the array of waitables by address. We actually sort a pairs so that we can
  // map back to the original index values later.
  StackList<WaiterAndIndex, 8> waitables;
  waitables.EnsureCapacity(count);

  for (int i = 0; i < count; ++i)
    waitables.Add(WaiterAndIndex { raw_waitables[i], i });

  ASSERT(count == waitables.size());

  waitables.Sort([](const WaiterAndIndex& lhs, const WaiterAndIndex& rhs) {
    return lhs.waitable < rhs.waitable;
  });

  // The set of waitables must be distinct. Since we have just sorted by
  // address, we can check this cheaply by comparing pairs of consecutive elements.
  for (int i = 0; i < waitables.size() - 1; ++i) {
    ASSERT(waitables[i].waitable != waitables[i+1].waitable);
  }

  SyncWaiter sw;

  const int r = EnqueueMany(&waitables[0], count, &sw);
  if (r) {
    // One of the events is already signaled. The SyncWaiter has not been
    // enqueued anywhere. EnqueueMany returns the count of remaining waitables
    // when the signaled one was seen, so the index of the signaled event is
    // @count - @r.
    return waitables[count - r].index;
  }

  // At this point, we hold the locks on all the WaitableEvents and we have
  // enqueued our waiter in them all.
  sw.lock()->Acquire();
    // Release the WaitableEvent locks in the reverse order
    for (int i = 0; i < count; ++i) {
      waitables[count - (1 + i)].waitable->kernel_->lock_.Release();
    }
    for (;;) {
      if (sw.fired())
        break;

      sw.cv()->Wait();
    }
  sw.lock()->Release();

  // The address of the WaitableEvent which fired is stored in the SyncWaiter.
  WaitableEvent* signaled_event = sw.signaling_event();
  // This will store the index of the raw_waitables which fired.
  int signaled_index = 0;

  // Take the locks of each WaitableEvent in turn (except the signaled one) and
  // remove our SyncWaiter from the wait-list
  for (int i = 0; i < count; ++i) {
    if (raw_waitables[i] != signaled_event) {
      raw_waitables[i]->kernel_->lock_.Acquire();
        // There's no possible ABA issue with the address of the SyncWaiter here
        // because it lives on the stack. Thus the tag value is just the pointer
        // value again.
        raw_waitables[i]->kernel_->Dequeue(&sw, &sw);
      raw_waitables[i]->kernel_->lock_.Release();
    } else {
      // By taking this lock here we ensure that |Signal| has completed by the
      // time we return, because |Signal| holds this lock. This matches the
      // behaviour of |Wait| and |TimedWait|.
      raw_waitables[i]->kernel_->lock_.Acquire();
      raw_waitables[i]->kernel_->lock_.Release();
      signaled_index = i;
    }
  }
  return signaled_index;
}

// If return value == 0:
//   The locks of the WaitableEvents have been taken in order and the Waiter has
//   been enqueued in the wait-list of each. None of the WaitableEvents are
//   currently signaled
// else:
//   None of the WaitableEvent locks are held. The Waiter has not been enqueued
//   in any of them and the return value is the index of the first WaitableEvent
//   which was signaled, from the end of the array.
int WaitableEvent::EnqueueMany(WaiterAndIndex* waitables, int count, Waiter* waiter) {
  if (!count)
    return 0;

  waitables[0].waitable->kernel_->lock_.Acquire();
  if (waitables[0].waitable->kernel_->signaled_) {
    if (!waitables[0].waitable->kernel_->manual_reset_)
      waitables[0].waitable->kernel_->signaled_ = false;
    waitables[0].waitable->kernel_->lock_.Release();
    return count;
  }

  const int r = EnqueueMany(waitables + 1, count - 1, waiter);
  if (r)
    waitables[0].waitable->kernel_->lock_.Release();
  else
    waitables[0].waitable->Enqueue(waiter);
  return r;
}

// Private functions...
WaitableEvent::WaitableEventKernel::WaitableEventKernel(
    ResetPolicy reset_policy,
    InitialState initial_state)
    : manual_reset_(reset_policy == ResetPolicy::Manual),
      signaled_(initial_state == InitialState::Signaled) {}

WaitableEvent::WaitableEventKernel::~WaitableEventKernel() = default;

// Wake all waiting waiters. Called with lock held.
bool WaitableEvent::SignalAll() {
  bool signaled_at_least_one = false;

  for (Waiter* waiter : kernel_->waiters_) {
    if (waiter->Fire(this))
      signaled_at_least_one = true;
  }

  kernel_->waiters_.Clear();
  return signaled_at_least_one;
}

// Try to wake a single waiter. Return true if one was woken. Called with lock held.
bool WaitableEvent::SignalOne() {
  for (;;) {
    if (kernel_->waiters_.IsEmpty())
      return false;

    const bool r = kernel_->waiters_.GetFirst()->Fire(this);
    kernel_->waiters_.RemoveAt(0);
    if (r)
      return true;
  }
}

// Add a waiter to the list of those waiting. Called with lock held.
void WaitableEvent::Enqueue(Waiter* waiter) {
  kernel_->waiters_.Add(waiter);
}

// Remove a waiter from the list of those waiting. Return true if the waiter was
// actually removed. Called with lock held.
bool WaitableEvent::WaitableEventKernel::Dequeue(Waiter* searched, void* tag) {
  for (int i = 0; i < waiters_.size(); ++i) {
    Waiter* waiter = waiters_[i];
    if (waiter == searched && waiter->Compare(tag)) {
      waiters_.RemoveAt(i);
      return true;
    }
  }
  return false;
}

} // namespace stp
