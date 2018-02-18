// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_THREAD_READWRITELOCK_H_
#define STP_BASE_THREAD_READWRITELOCK_H_

#include "Base/Compiler/Os.h"
#include "Base/Debug/Assert.h"

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#elif OS(POSIX)
#include <pthread.h>
#else
#error "no R/W lock defined for this platform"
#endif

namespace stp {

// An OS-independent wrapper around reader-writer locks. There's no magic here.
//
// You are strongly encouraged to use Lock instead of this, unless you
// can demonstrate contention and show that this would lead to an improvement.
// This lock does not make any guarantees of fairness, which can lead to writer
// starvation under certain access patterns. You should carefully consider your
// writer access patterns before using this lock.
class BASE_EXPORT BasicReadWriteLock {
 public:
  // Reader lock functions.
  void ReadAcquire();
  void Readrelease();

  // Writer lock functions.
  void WriteAcquire();
  void Writerelease();

  void Init();
  void Fini();

  #if OS(WIN)
  using NativeHandle = SRWLOCK;
  #elif OS(POSIX)
  using NativeHandle = pthread_rwlock_t;
  #endif

  NativeHandle native_handle_;
};

class BASE_EXPORT ReadWriteLock : public BasicReadWriteLock {
 public:
  ReadWriteLock() { Init(); }
  ~ReadWriteLock() { Fini(); }

  DISALLOW_COPY_AND_ASSIGN(ReadWriteLock);
};

class AutoReadLock {
 public:
  explicit AutoReadLock(BasicReadWriteLock& lock) : lock_(lock) {
    lock_.ReadAcquire();
  }
  ~AutoReadLock() {
    lock_.Readrelease();
  }

 private:
  BasicReadWriteLock& lock_;
  DISALLOW_COPY_AND_ASSIGN(AutoReadLock);
};

class AutoWriteLock {
 public:
  explicit AutoWriteLock(BasicReadWriteLock& lock) : lock_(lock) {
    lock_.WriteAcquire();
  }
  ~AutoWriteLock() {
    lock_.Writerelease();
  }

 private:
  BasicReadWriteLock& lock_;
  DISALLOW_COPY_AND_ASSIGN(AutoWriteLock);
};

#if OS(WIN)

#define BASIC_READ_WRITE_LOCK_INITIALIZER { SRWLOCK_INIT }

inline void BasicReadWriteLock::Init() {
  ::InitializeSRWLock(&native_handle_);
}

inline void BasicReadWriteLock::Fini() {
  // No need to uninitialize R/W lock on windows.
}

inline void BasicReadWriteLock::ReadAcquire() {
  ::AcquireSRWLockShared(&native_handle_);
}

inline void BasicReadWriteLock::Readrelease() {
  ::ReleaseSRWLockShared(&native_handle_);
}

inline void BasicReadWriteLock::WriteAcquire() {
  ::AcquireSRWLockExclusive(&native_handle_);
}

inline void BasicReadWriteLock::Writerelease() {
  ::ReleaseSRWLockExclusive(&native_handle_);
}

#elif OS(POSIX)

#define BASIC_READ_WRITE_LOCK_INITIALIZER { PTHREAD_RWLOCK_INITIALIZER }

inline void BasicReadWriteLock::Init() {
  int result = pthread_rwlock_init(&native_handle_, nullptr);
  ASSERT_UNUSED(result == 0, result);
}

inline void BasicReadWriteLock::Fini() {
  int result = pthread_rwlock_destroy(&native_handle_);
  ASSERT_UNUSED(result == 0, result);
}

inline void BasicReadWriteLock::ReadAcquire() {
  int result = pthread_rwlock_rdlock(&native_handle_);
  ASSERT_UNUSED(result == 0, result);
}

inline void BasicReadWriteLock::Readrelease() {
  int result = pthread_rwlock_unlock(&native_handle_);
  ASSERT_UNUSED(result == 0, result);
}

inline void BasicReadWriteLock::WriteAcquire() {
  int result = pthread_rwlock_wrlock(&native_handle_);
  ASSERT_UNUSED(result == 0, result);
}

inline void BasicReadWriteLock::Writerelease() {
  int result = pthread_rwlock_unlock(&native_handle_);
  ASSERT_UNUSED(result == 0, result);
}
#endif

} // namespace stp

#endif // STP_BASE_THREAD_READWRITELOCK_H_
