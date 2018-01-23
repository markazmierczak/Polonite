// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Sync/OneWriterSeqLock.h"

namespace stp {

subtle::Atomic32 BasicOneWriterSeqLock::ReadBegin() {
  subtle::Atomic32 version;
  for (;;) {
    version = subtle::NoBarrier_Load(&sequence_);

    // If the counter is even, then the associated data might be in a
    // consistent state, so we can try to read.
    if ((version & 1) == 0)
      break;

    // Otherwise, the writer is in the middle of an update. Retry the read.
    NativeThread::Yield();
  }
  return version;
}

bool BasicOneWriterSeqLock::ReadRetry(subtle::Atomic32 version) {
  // If the sequence number was updated then a read should be re-attempted.
  // -- Load fence, read membarrier
  return subtle::Release_Load(&sequence_) != version;
}

void BasicOneWriterSeqLock::WriteBegin() {
  // Increment the sequence number to odd to indicate the beginning of a write update.
  subtle::Barrier_AtomicIncrement(&sequence_, 1);
  // -- Store fence, write membarrier
}

void BasicOneWriterSeqLock::WriteEnd() {
  // Increment the sequence to an even number to indicate the completion of a write update.
  // -- Store fence, write membarrier
  subtle::Barrier_AtomicIncrement(&sequence_, 1);
}

} // namespace stp
