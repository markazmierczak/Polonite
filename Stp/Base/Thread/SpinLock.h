// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_THREAD_SPINLOCK_H_
#define STP_BASE_THREAD_SPINLOCK_H_

#include "Base/Debug/Assert.h"
#include "Base/Thread/AtomicOps.h"

// Spinlock is a simple spinlock class based on the standard CPU primitive of
// atomic increment and decrement of an int at a given memory address.
// These are intended only for very short duration locks and assume a system with multiple cores.
// For any potentially longer wait you should use a real lock, such as |Lock|.
//
// |SpinLock|s MUST be globals. Using them as (e.g.) struct/class members will
// result in an uninitialized lock, which is dangerously incorrect.

namespace stp {

#define BASIC_SPIN_LOCK_INITIALIZER { 0 }

class BasicSpinLock {
 public:
  ALWAYS_INLINE bool tryAcquire() {
    return subtle::Acquire_CompareAndSwap(&lock_, Free, Held) == Free;
  }

  ALWAYS_INLINE void Acquire() {
    if (UNLIKELY(!tryAcquire()))
      AcquireSlow();
  }

  // Releases the lock.
  // The lock must be held by calling thread.
  ALWAYS_INLINE void release() {
    subtle::Release_Store(&lock_, Free);
  }

  bool IsHeld() const {
    return subtle::NoBarrier_Load(&lock_) != Free;
  }

  void assertAcquired() {
    ASSERT(IsHeld());
  }

  volatile subtle::Atomic32 lock_;

 protected:
  enum State : subtle::Atomic32 {
    Free = 0,
    Held = 1,
  };

  // This is called if the initial attempt to acquire the lock fails. It's
  // slower, but has a much better scheduling and power consumption behavior.
  BASE_EXPORT void AcquireSlow();
};

class SpinLock : public BasicSpinLock {
 public:
  SpinLock() { lock_ = Free; }

 private:
  using BasicSpinLock::lock_;

  DISALLOW_COPY_AND_ASSIGN(SpinLock);
};

class AutoSpinLock {
 public:
  struct AlreadyAcquired {};

  explicit AutoSpinLock(SpinLock& lock) : lock_(lock) {
    lock_.Acquire();
  }

  AutoSpinLock(SpinLock& lock, AlreadyAcquired) : lock_(lock) {
    lock_.assertAcquired();
  }

  ~AutoSpinLock() {
    lock_.assertAcquired();
    lock_.release();
  }

 private:
  SpinLock& lock_;

  DISALLOW_COPY_AND_ASSIGN(AutoSpinLock);
};

} // namespace stp

#endif // STP_BASE_THREAD_SPINLOCK_H_
