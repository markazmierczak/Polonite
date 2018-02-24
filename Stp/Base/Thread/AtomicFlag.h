// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_THREAD_ATOMICFLAG_H_
#define STP_BASE_THREAD_ATOMICFLAG_H_

#include "Base/Thread/AtomicOps.h"

namespace stp {

// A flag that can safely be set from one thread and read from other threads.
//
// This class IS NOT intended for synchronization between threads.
struct BASE_EXPORT AtomicFlag {
  constexpr AtomicFlag() {}

  // Set the flag. Must always be called from the same sequence.
  void Set();

  // Returns true iff the flag was set. If this returns true, the current thread
  // is guaranteed to be synchronized with all memory operations on the sequence
  // which invoked Set() up until at least the first call to Set() on it.
  bool IsSet() const;

  // Resets the flag. Be careful when using this: callers might not expect
  // IsSet() to return false after returning true once.
  void UnsafeResetForTesting();

  subtle::Atomic32 flag_ = 0;
};

inline void AtomicFlag::Set() {
  subtle::Release_Store(&flag_, 1);
}

inline bool AtomicFlag::IsSet() const {
  return subtle::Acquire_Load(&flag_) != 0;
}

inline void AtomicFlag::UnsafeResetForTesting() {
  subtle::Release_Store(&flag_, 0);
}

} // namespace stp

#endif // STP_BASE_THREAD_ATOMICFLAG_H_
