// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_THREAD_READWRITELOCK_H_
#define STP_BASE_THREAD_READWRITELOCK_H_

#include "Base/Compiler/Os.h"
#include "Base/Debug/Assert.h"

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#elif OS(POSIX)
#include <pthread.h>
#endif

namespace stp {

class BasicReadWriteLock {
 public:
  void readAcquire();
  void readRelease();

  void writeAcquire();
  void writeRelease();

  void init();
  void fini();

  #if OS(WIN)
  using NativeHandle = SRWLOCK;
  #elif OS(POSIX)
  using NativeHandle = pthread_rwlock_t;
  #endif

  NativeHandle native_handle_;
};

class ReadWriteLock : public BasicReadWriteLock {
 public:
  ReadWriteLock() { init(); }
  ~ReadWriteLock() { fini(); }

  DISALLOW_COPY_AND_ASSIGN(ReadWriteLock);
};

class AutoReadLock {
 public:
  explicit AutoReadLock(BasicReadWriteLock& lock) : lock_(lock) {
    lock_.readAcquire();
  }
  ~AutoReadLock() {
    lock_.readRelease();
  }
  DISALLOW_COPY_AND_ASSIGN(AutoReadLock);

 private:
  BasicReadWriteLock& lock_;
};

class AutoWriteLock {
 public:
  explicit AutoWriteLock(BasicReadWriteLock& lock) : lock_(lock) {
    lock_.writeAcquire();
  }
  ~AutoWriteLock() {
    lock_.writeRelease();
  }
  DISALLOW_COPY_AND_ASSIGN(AutoWriteLock);

 private:
  BasicReadWriteLock& lock_;
};

#if OS(WIN)

#define BASIC_READ_WRITE_LOCK_INITIALIZER { SRWLOCK_INIT }

inline void BasicReadWriteLock::init() {
  ::InitializeSRWLock(&native_handle_);
}

inline void BasicReadWriteLock::fini() {
  // No need to uninitialize R/W lock on windows.
}

inline void BasicReadWriteLock::readAcquire() {
  ::AcquireSRWLockShared(&native_handle_);
}

inline void BasicReadWriteLock::readRelease() {
  ::ReleaseSRWLockShared(&native_handle_);
}

inline void BasicReadWriteLock::writeAcquire() {
  ::AcquireSRWLockExclusive(&native_handle_);
}

inline void BasicReadWriteLock::writeRelease() {
  ::ReleaseSRWLockExclusive(&native_handle_);
}

#elif OS(POSIX)

#define BASIC_READ_WRITE_LOCK_INITIALIZER { PTHREAD_RWLOCK_INITIALIZER }

inline void BasicReadWriteLock::init() {
  int result = pthread_rwlock_init(&native_handle_, nullptr);
  ASSERT(result == 0);
}

inline void BasicReadWriteLock::fini() {
  int result = pthread_rwlock_destroy(&native_handle_);
  ASSERT(result == 0);
}

inline void BasicReadWriteLock::readAcquire() {
  int result = pthread_rwlock_rdlock(&native_handle_);
  ASSERT(result == 0);
}

inline void BasicReadWriteLock::readRelease() {
  int result = pthread_rwlock_unlock(&native_handle_);
  ASSERT(result == 0);
}

inline void BasicReadWriteLock::writeAcquire() {
  int result = pthread_rwlock_wrlock(&native_handle_);
  ASSERT(result == 0);
}

inline void BasicReadWriteLock::writeRelease() {
  int result = pthread_rwlock_unlock(&native_handle_);
  ASSERT(result == 0);
}
#endif

} // namespace stp

#endif // STP_BASE_THREAD_READWRITELOCK_H_
