// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_THREAD_LOCK_H_
#define STP_BASE_THREAD_LOCK_H_

#include "Base/Thread/NativeLock.h"
#include "Base/Thread/NativeThread.h"

namespace stp {

#if ASSERT_IS_ON
#define BASIC_LOCK_INITIALIZER { NATIVE_LOCK_INITIALIZER, stp::InvalidNativeThreadHandle }
#else
#define BASIC_LOCK_INITIALIZER { NATIVE_LOCK_INITIALIZER }
#endif

class BasicLock {
 public:
  BASE_EXPORT bool tryAcquire();

  BASE_EXPORT void acquire();
  BASE_EXPORT void release();

  BASE_EXPORT void assertAcquired() const;

  NativeLockObject native_object_;

  #if ASSERT_IS_ON
  void checkHeldAndUnmark();
  void checkUnheldAndMark();

  NativeThreadHandle owning_thread_;
  #endif
};

class BASE_EXPORT Lock : public BasicLock {
  DISALLOW_COPY_AND_ASSIGN(Lock);
 public:
  Lock() {
    NativeLock::init(&native_object_);
    #if ASSERT_IS_ON
    owning_thread_ = InvalidNativeThreadHandle;
    #endif
  }

  ~Lock() {
    #if ASSERT_IS_ON
    ASSERT(owning_thread_ == InvalidNativeThreadHandle);
    #endif
    NativeLock::fini(&native_object_);
  }

  using BasicLock::tryAcquire;
  using BasicLock::acquire;
  using BasicLock::release;

 private:
  using BasicLock::native_object_;
};

class AutoLock {
  DISALLOW_COPY_AND_ASSIGN(AutoLock);
 public:
  enum AlreadyAcquiredTag { AlreadyAcquired };

  explicit AutoLock(Borrow<BasicLock> lock) : lock_(lock) {
    lock_.acquire();
  }

  AutoLock(BasicLock& lock, AlreadyAcquiredTag) : lock_(lock) {
    lock_.assertAcquired();
  }

  ~AutoLock() {
    lock_.assertAcquired();
    lock_.release();
  }

 private:
  BasicLock& lock_;
};

class AutoUnlock {
  DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
 public:
  explicit AutoUnlock(BasicLock* lock) : lock_(*lock) {
    lock_.assertAcquired();
    lock_.release();
  }

  ~AutoUnlock() {
    lock_.acquire();
  }

 private:
  BasicLock& lock_;
};

#if !ASSERT_IS_ON
inline bool BasicLock::tryAcquire() { return NativeLock::tryAcquire(&native_object_); }
inline void BasicLock::acquire() { NativeLock::acquire(&native_object_); }
inline void BasicLock::release() { NativeLock::release(&native_object_); }
inline void BasicLock::assertAcquired() const {}
#endif

} // namespace stp

#endif // STP_BASE_THREAD_LOCK_H_
