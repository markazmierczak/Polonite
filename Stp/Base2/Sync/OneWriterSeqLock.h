// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_SYNC_ONEWRITERSEQLOCK_H_
#define STP_BASE_SYNC_ONEWRITERSEQLOCK_H_

#include "Base/Sync/AtomicOps.h"
#include "Base/Thread/NativeThread.h"

namespace stp {

// This SeqLock handles only *one* writer and multiple readers. It may be
// suitable for low-contention with relatively infrequent writes, and many readers. See:
//   http://en.wikipedia.org/wiki/Seqlock
//   http://www.concurrencykit.org/doc/ck_sequence.html
// This implementation is based on ck_sequence.h from http://concurrencykit.org.
//
// You must be very careful not to operate on potentially inconsistent read
// buffers. If the read must be retry'd, the data in the read buffer could
// contain any random garbage. e.g., contained pointers might be
// garbage, or indices could be out of range. Probably the only suitable thing
// to do during the read loop is to make a copy of the data, and operate on it
// only after the read was found to be consistent.

#define BSAIC_ONE_WRITER_SEQ_LOCK_INITIALIZER { 0 }

class BASE_EXPORT BasicOneWriterSeqLock {
 public:
  subtle::Atomic32 ReadBegin();
  bool ReadRetry(subtle::Atomic32 version);

  void WriteBegin();
  void WriteEnd();

  subtle::Atomic32 sequence_;
};

class OneWriterSeqLock : public BasicOneWriterSeqLock {
 public:
  OneWriterSeqLock() { sequence_ = 0; }

 private:
  using BasicOneWriterSeqLock::sequence_;

  DISALLOW_COPY_AND_ASSIGN(OneWriterSeqLock);
};

} // namespace stp

#endif // STP_BASE_SYNC_ONEWRITERSEQLOCK_H_
