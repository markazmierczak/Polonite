// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_THREAD_NATIVELOCK_H_
#define STP_BASE_THREAD_NATIVELOCK_H_

#include "Base/Compiler/Os.h"
#include "Base/Debug/Assert.h"

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#elif OS(POSIX)
#include <errno.h>
#include <pthread.h>
#endif

namespace stp {

#if OS(WIN)
// SRWLOCK is generally faster than CRITICAL_SECTION.
using NativeLockObject = SRWLOCK;
#elif OS(POSIX)
using NativeLockObject = pthread_mutex_t;
#endif

class NativeLock {
  STATIC_ONLY(NativeLock);
 public:
  static void init(NativeLockObject* object);
  static void fini(NativeLockObject* object);
  static bool tryAcquire(NativeLockObject* object);
  static void acquire(NativeLockObject* object);
  static void release(NativeLockObject* object);
};

#if OS(WIN)

#define NATIVE_LOCK_INITIALIZER SRWLOCK_INIT

inline void NativeLock::init(NativeLockObject* object) {
  InitializeSRWLock(object);
}

inline void NativeLock::fini(NativeLockObject* object) {}

inline bool NativeLock::tryAcquire(NativeLockObject* object) {
  return !!::TryAcquireSRWLockExclusive(object);
}

inline void NativeLock::acquire(NativeLockObject* object) {
  ::AcquireSRWLockExclusive(object);
}

inline void NativeLock::release(NativeLockObject* object) {
  ::ReleaseSRWLockExclusive(object);
}

#elif OS(POSIX)

#define NATIVE_LOCK_INITIALIZER PTHREAD_MUTEX_INITIALIZER

inline void NativeLock::init(NativeLockObject* object) {
  #if ASSERT_IS_ON
  // In debug, setup attributes for lock error checking.
  pthread_mutexattr_t mta;
  int rv = pthread_mutexattr_init(&mta);
  ASSERT(rv == 0);
  rv = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
  ASSERT(rv == 0);
  rv = pthread_mutex_init(object, &mta);
  ASSERT(rv == 0);
  rv = pthread_mutexattr_destroy(&mta);
  ASSERT(rv == 0);
  #else
  // In release, go with the default lock attributes.
  pthread_mutex_init(object, NULL);
  #endif
}

inline void NativeLock::fini(NativeLockObject* object) {
  int rv = pthread_mutex_destroy(object);
  ASSERT(rv == 0);
}

inline bool NativeLock::tryAcquire(NativeLockObject* object) {
  int rv = pthread_mutex_trylock(object);
  ASSERT(rv == 0 || rv == EBUSY);
  return rv == 0;
}

inline void NativeLock::acquire(NativeLockObject* object) {
  int rv = pthread_mutex_lock(object);
  ASSERT(rv == 0);
}

inline void NativeLock::release(NativeLockObject* object) {
  int rv = pthread_mutex_unlock(object);
  ASSERT(rv == 0);
}
#endif // OS(*)

} // namespace stp

#endif // STP_BASE_THREAD_NATIVELOCK_H_
