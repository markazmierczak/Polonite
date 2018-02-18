// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Util/LazyInstance.h"

#include "Base/App/AtExit.h"
#include "Base/Thread/NativeThread.h"

namespace stp {
namespace detail {

bool NeedsLazyInstance(subtle::AtomicWord* state) {
  // Try to create the instance, if we're the first, will go from 0 to
  // LazyInstanceStateCreating, otherwise we've already been beaten here.
  // The memory access has no memory ordering as state 0 and
  // LazyInstanceStateCreating have no associated data (memory barriers are
  // all about ordering of memory accesses to *associated* data).
  if (subtle::NoBarrier_CompareAndSwap(state, 0, LazyInstanceStateCreating) == 0) {
    // Caller must create instance
    return true;
  }

  // It's either in the process of being created, or already created. Spin.
  // The load has acquire memory ordering as a thread which sees
  // state_ == Created needs to acquire visibility over
  // the associated data (buf_). Pairing Release_Store is in
  // CompleteLazyInstance().
  while (subtle::Acquire_Load(state) == LazyInstanceStateCreating)
    NativeThread::Yield();

  // Someone else created the instance.
  return false;
}

void CompleteLazyInstance(
    subtle::AtomicWord* state, subtle::AtomicWord new_instance,
    void* lazy_instance, void (*dtor)(void*)) {
  // Instance is created, go from Creating to Created.
  // Releases visibility over private_buf_ to readers. Pairing Acquire_Load's
  // are in NeedsInstance() and Pointer().
  subtle::Release_Store(state, new_instance);

  // Make sure that the lazily instantiated object will get destroyed at exit.
  if (dtor)
    AtExitManager::registerCallback(dtor, lazy_instance);
}

} // namespace detail
} // namespace stp
