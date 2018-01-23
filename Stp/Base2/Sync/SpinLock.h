// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_SYNC_SPINLOCK_H_
#define STP_BASE_SYNC_SPINLOCK_H_

#include "Base/Debug/Assert.h"
#include "Base/Sync/AtomicOps.h"

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
  ALWAYS_INLINE bool TryAcquire() {
    return subtle::Acquire_CompareAndSwap(&lock_, Free, Held) == Free;
  }

  ALWAYS_INLINE void Acquire() {
    if (UNLIKELY(!TryAcquire()))
      AcquireSlow();
  }

  // Releases the lock.
  // The lock must be held by calling thread.
  ALWAYS_INLINE void Release() {
    subtle::Release_Store(&lock_, Free);
  }

  bool IsHeld() const {
    return subtle::NoBarrier_Load(&lock_) != Free;
  }

  void AssertAcquired() {
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
    lock_.AssertAcquired();
  }

  ~AutoSpinLock() {
    lock_.AssertAcquired();
    lock_.Release();
  }

 private:
  SpinLock& lock_;

  DISALLOW_COPY_AND_ASSIGN(AutoSpinLock);
};

} // namespace stp

#endif // STP_BASE_SYNC_SPINLOCK_H_
