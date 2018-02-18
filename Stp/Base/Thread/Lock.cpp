// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/Lock.h"

#if ASSERT_IS_ON

namespace stp {

/**
 * @defgroup BaseLocksGroup
 *
 * @ref Lock and @ref BasicLock are simple wrappers around platform-specific spin-lock mechanism.
 *
 * The wrappers provide additional functionality used in debug mode: @ref Lock::assertAcquired().
 *
 * @note
 * Only use BasicLock directly (with BASIC_LOCK_INITIALIZER) when global lock is needed.
 * Otherwise use Lock class.
 */

/**
 * @fn Lock::tryAcquire
 * If the lock is not held, take it and return true. If the lock is already
 * held by another thread, immediately return false. This must not be called
 * by a thread already holding the lock (undefined behavior).
 */
bool BasicLock::tryAcquire() {
  bool rv = NativeLock::tryAcquire(&native_object_);
  if (rv)
    checkUnheldAndMark();

  return rv;
}

/**
 * @fn Lock::acquire
 * Take the lock, blocking until it is available if necessary.
 *
 * @note
 * We do not permit recursive locks and will commonly fire a ASSERT() if
 * a thread attempts to acquire the lock a second time (while already holding it).
 */
void BasicLock::acquire() {
  NativeLock::acquire(&native_object_);
  checkUnheldAndMark();
}

/**
 * @fn Lock::release
 * Release the lock.
 * This must only be called by the lock's holder: after a successful call to
 * @ref BasicLock::tryAcquire(), or a call to @ref BasicLock::acquire().
 *
 */
void BasicLock::release() {
  checkHeldAndUnmark();
  NativeLock::release(&native_object_);
}

/**
 * @fn Lock::assertAcquired
 * Requires the lock to be held by the current thread.
 */
void BasicLock::assertAcquired() const {
  ASSERT(owning_thread_ == NativeThread::currentHandle());
}

// Members and routines taking care of locks assertions.
// Note that this checks for recursive locks and allows them if the variable is set.
// This is allowed by the underlying implementation on windows but not on POSIX,
// so we're doing unneeded checks on POSIX. It's worth it to share the code.

/** @internal */
void BasicLock::checkHeldAndUnmark() {
  ASSERT(owning_thread_ == NativeThread::currentHandle());
  owning_thread_ = InvalidNativeThreadHandle;
}

/** @internal */
void BasicLock::checkUnheldAndMark() {
  ASSERT(owning_thread_ == InvalidNativeThreadHandle);
  owning_thread_ = NativeThread::currentHandle();
}

/** @class AutoUnlock
 * AutoUnlock is a helper that will @ref Lock::release() the |lock| argument in the
 * constructor, and re-acquire() it in the destructor.
 */

} // namespace stp

#endif // ASSERT_IS_ON
