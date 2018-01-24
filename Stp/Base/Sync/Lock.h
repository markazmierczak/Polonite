// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_SYNC_LOCK_H_
#define STP_BASE_SYNC_LOCK_H_

#include "Base/Sync/NativeLock.h"
#include "Base/Thread/NativeThread.h"

namespace stp {

#if ASSERT_IS_ON()
#define BASIC_LOCK_INITIALIZER { \
    NATIVE_LOCK_INITIALIZER, \
    stp::InvalidNativeThreadHandle \
  }
#else
#define BASIC_LOCK_INITIALIZER { NATIVE_LOCK_INITIALIZER }
#endif

// This class implements the underlying platform-specific spin-lock mechanism
// used for the Lock class.
//
// The only real intelligence in this class is in debug mode for the support for the
// AssertAcquired() method.
//
// NOTE: Only use BasicLock directly (with BASIC_LOCK_INITIALIZER) when global lock is needed.
//       Otherwise use Lock class.
class BASE_EXPORT BasicLock {
 public:
  // If the lock is not held, take it and return true. If the lock is already
  // held by another thread, immediately return false. This must not be called
  // by a thread already holding the lock (what happens is undefined and an
  // assertion may fail).
  bool TryAcquire();

  // Take the lock, blocking until it is available if necessary.
  //
  // NOTE: We do not permit recursive locks and will commonly fire a ASSERT() if
  // a thread attempts to acquire the lock a second time (while already holding it).
  void Acquire();

  // Release the lock.  This must only be called by the lock's holder: after
  // a successful call to Try, or a call to Lock.
  void Release();

  void AssertAcquired() const;

  NativeLockObject native_object_;

  #if ASSERT_IS_ON()
  // Members and routines taking care of locks assertions.
  // Note that this checks for recursive locks and allows them if the variable is set.
  // This is allowed by the underlying implementation on windows but not on POSIX,
  // so we're doing unneeded checks on POSIX. It's worth it to share the code.
  void CheckHeldAndUnmark();
  void CheckUnheldAndMark();

  NativeThreadHandle owning_thread_;
  #endif
};

class BASE_EXPORT Lock : public BasicLock {
 public:
  Lock() {
    NativeLock::Init(&native_object_);
    #if ASSERT_IS_ON()
    owning_thread_ = InvalidNativeThreadHandle;
    #endif
  }

  ~Lock() {
    ASSERT(owning_thread_ == InvalidNativeThreadHandle);
    NativeLock::Fini(&native_object_);
  }

  using BasicLock::TryAcquire;
  using BasicLock::Acquire;
  using BasicLock::Release;

 private:
  using BasicLock::native_object_;

  DISALLOW_COPY_AND_ASSIGN(Lock);
};

class AutoLock {
 public:
  struct AlreadyAcquired {};

  explicit AutoLock(BasicLock* lock) : lock_(*lock) {
    lock_.Acquire();
  }

  AutoLock(BasicLock& lock, AlreadyAcquired) : lock_(lock) {
    lock_.AssertAcquired();
  }

  ~AutoLock() {
    lock_.AssertAcquired();
    lock_.Release();
  }

 private:
  BasicLock& lock_;

  DISALLOW_COPY_AND_ASSIGN(AutoLock);
};

// AutoUnlock is a helper that will Release() the |lock| argument in the
// constructor, and re-Acquire() it in the destructor.
class AutoUnlock {
 public:
  explicit AutoUnlock(BasicLock* lock) : lock_(*lock) {
    // We require our caller to have the lock.
    lock_.AssertAcquired();
    lock_.Release();
  }

  ~AutoUnlock() {
    lock_.Acquire();
  }

 private:
  BasicLock& lock_;
  DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
};

#if !ASSERT_IS_ON()
inline bool BasicLock::TryAcquire() { return NativeLock::TryAcquire(&native_object_); }
inline void BasicLock::Acquire() { NativeLock::Acquire(&native_object_); }
inline void BasicLock::Release() { NativeLock::Release(&native_object_); }
inline void BasicLock::AssertAcquired() const {}
#endif

} // namespace stp

#endif // STP_BASE_SYNC_LOCK_H_
