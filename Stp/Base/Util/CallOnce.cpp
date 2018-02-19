// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/CallOnce.h"

#include "Base/Thread/NativeThread.h"

namespace stp {

bool CallOnce::needsCall(subtle::Atomic32* state) {
  if (subtle::NoBarrier_CompareAndSwap(state, NotStarted, Claimed) == 0)
    return true;

  while (subtle::Acquire_Load(state) == Claimed)
    NativeThread::Yield();

  return false;
}

} // namespace stp
