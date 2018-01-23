// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_THREAD_THREADCHECKER_H_
#define STP_BASE_THREAD_THREADCHECKER_H_

#include "Base/Debug/Assert.h"
#include "Base/Sync/Lock.h"
#include "Base/Thread/NativeThread.h"

// ThreadChecker is a helper class used to help verify that some methods of a
// class are called from the same thread.
//
// Example:
// class MyClass {
//  public:
//   void Foo() {
//     ASSERT(thread_checker_.CalledOnValidThread());
//     ... (do stuff) ...
//   }
//
//  private:
//   ThreadChecker thread_checker_;
// }
//
// In Release mode, CalledOnValidThread will always return true.

namespace stp {

#if ASSERT_IS_ON()

class BASE_EXPORT ThreadChecker {
 public:
  ThreadChecker();
  ~ThreadChecker();

  bool CalledOnValidThread() const WARN_UNUSED_RESULT;

  // Changes the thread that is checked for in CalledOnValidThread.  This may
  // be useful when an object may be created on one thread and then used
  // exclusively on another thread.
  void DetachFromThread();

 private:
  void EnsureThreadIdAssigned() const;

  mutable Lock lock_;
  // This is mutable so that CalledOnValidThread can set it.
  // It's guarded by |lock_|.
  mutable NativeThreadHandle valid_thread_ = InvalidNativeThreadHandle;
};

#else

// Do nothing implementation, for use in release mode.
//
// Note: You should almost always use the ThreadChecker class to get the
// right version for your build configuration.
class ThreadChecker {
 public:
  bool CalledOnValidThread() const WARN_UNUSED_RESULT {
    return true;
  }

  void DetachFromThread() {}
};

#endif // ASSERT_IS_ON()

} // namespace stp

#endif // STP_BASE_THREAD_THREADCHECKER_H_
