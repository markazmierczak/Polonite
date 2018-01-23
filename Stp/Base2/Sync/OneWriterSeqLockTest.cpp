// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Sync/OneWriterSeqLock.h"

#include "Base/Sync/AtomicRefCount.h"
#include "Base/Test/GTest.h"
#include "Base/Thread/Thread.h"

namespace stp {

// Basic test to make sure that basic operation works correctly.

struct TestData {
  unsigned a, b, c;
};

class BasicSeqLockTestThread : public Thread {
 public:
  BasicSeqLockTestThread() {}

  void Init(OneWriterSeqLock* seqlock, TestData* data, subtle::AtomicWord* ready) {
    seqlock_ = seqlock;
    data_ = data;
    ready_ = ready;
  }

  int Main() override {
    while (AtomicRefCountIsZero(ready_))
      ThisThread::Yield();

    for (unsigned i = 0; i < 1000; ++i) {
      TestData copy;
      subtle::AtomicWord version;
      do {
        version = seqlock_->ReadBegin();
        copy = *data_;
      } while (seqlock_->ReadRetry(version));

      EXPECT_EQ(copy.a + 100, copy.b);
      EXPECT_EQ(copy.c, copy.b + copy.a);
    }
    AtomicRefCountDec(ready_);
    return 0;
  }

 private:
  OneWriterSeqLock* seqlock_ = nullptr;
  TestData* data_ = nullptr;
  AtomicRefCount* ready_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(BasicSeqLockTestThread);
};

TEST(OneWriterSeqLockTest, ManyThreads) {
  OneWriterSeqLock seqlock;
  TestData data = { 0, 0, 0 };
  AtomicRefCount ready = 0;

  constexpr int NumReaderThreads = 10;
  BasicSeqLockTestThread threads[NumReaderThreads];

  for (int i = 0; i < NumReaderThreads; ++i)
    threads[i].Init(&seqlock, &data, &ready);
  for (int i = 0; i < NumReaderThreads; ++i)
    threads[i].Start();

  // The main thread is the writer, and the spawned are readers.
  unsigned counter = 0;
  for (;;) {
    seqlock.WriteBegin();
    data.a = counter++;
    data.b = data.a + 100;
    data.c = data.b + data.a;
    seqlock.WriteEnd();

    if (counter == 1)
      AtomicRefCountIncN(&ready, NumReaderThreads);

    if (AtomicRefCountIsZero(&ready))
      break;
  }

  for (int i = 0; i < NumReaderThreads; ++i)
    threads[i].Join();
}

} // namespace stp
