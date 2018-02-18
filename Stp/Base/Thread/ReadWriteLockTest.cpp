// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/ReadWriteLock.h"

#include "Base/Test/GTest.h"
#include "Base/Thread/Thread.h"
#include "Base/Thread/WaitableEvent.h"

#include <stdlib.h>

namespace stp {

class BasicReadWriteLockTestThread : public Thread {
 public:
  explicit BasicReadWriteLockTestThread(ReadWriteLock* lock)
      : lock_(lock), acquired_(0) {}

  int Main() override {
    for (int i = 0; i < 10; i++) {
      AutoReadLock locker(*lock_);
      acquired_++;
    }
    for (int i = 0; i < 10; i++) {
      AutoWriteLock locker(*lock_);
      acquired_++;
      ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
    }
    return 0;
  }

  int getAcquired() const { return acquired_; }

 private:
  ReadWriteLock* lock_;
  int acquired_;
};

TEST(ReadWriteLockTest, Basic) {
  ReadWriteLock lock;
  BasicReadWriteLockTestThread thread(&lock);
  thread.Start();

  int acquired = 0;
  for (int i = 0; i < 5; i++) {
    AutoReadLock locker(lock);
    acquired++;
  }
  for (int i = 0; i < 10; i++) {
    AutoWriteLock locker(lock);
    acquired++;
    ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
  }
  for (int i = 0; i < 5; i++) {
    AutoReadLock locker(lock);
    acquired++;
  }

  thread.Join();

  EXPECT_EQ(20, acquired);
  EXPECT_GE(20, thread.getAcquired());
}

class ReaderReadWriteLockTestThread : public Thread {
 public:
  ReaderReadWriteLockTestThread(ReadWriteLock* lock) : lock_(lock) {}

  int Main() override {
    AutoReadLock locker(*lock_);
    did_acquire_ = true;
    return 0;
  }

  bool didAcquire() const { return did_acquire_; }

 private:
  ReadWriteLock* lock_;
  bool did_acquire_ = false;
};

// Tests that reader locks allow multiple simultaneous reader acquisitions.
TEST(ReadWriteLockTest, ReaderTwoThreads) {
  ReadWriteLock lock;

  AutoReadLock auto_lock(lock);

  ReaderReadWriteLockTestThread thread(&lock);
  thread.Start();
  thread.Join();
  EXPECT_TRUE(thread.didAcquire());
}

class ReadAndWriteReadWriteLockTestThread : public Thread {
 public:
  ReadAndWriteReadWriteLockTestThread(ReadWriteLock* lock, int* value)
      : lock_(lock),
        value_(value),
        event_(WaitableEvent::ResetPolicy::Manual,
               WaitableEvent::InitialState::NotSignaled) {}

  int Main() override {
    AutoWriteLock locker(*lock_);
    (*value_)++;
    event_.Signal();
    return 0;
  }

  void wait() { event_.Wait(); }

 private:
  ReadWriteLock* lock_;
  int* value_;
  WaitableEvent event_;
};

// Tests that writer locks exclude reader locks.
TEST(ReadWriteLockTest, ReadAndWriteThreads) {
  ReadWriteLock lock;
  int value = 0;

  ReadAndWriteReadWriteLockTestThread thread(&lock, &value);
  {
    AutoReadLock read_locker(lock);
    thread.Start();

    ThisThread::SleepFor(TimeDelta::FromMilliseconds(10));

    // |value| should be unchanged since we hold a reader lock.
    EXPECT_EQ(0, value);
  }

  thread.wait();
  // After releasing our reader lock, the thread can acquire a write lock and
  // change |value|.
  EXPECT_EQ(1, value);
  thread.Join();
}

class WriterReadWriteLockTestThread : public Thread {
 public:
  WriterReadWriteLockTestThread(ReadWriteLock* lock, int* value)
      : lock_(lock), value_(value) {}

  // Static helper which can also be called from the main thread.
  static void doStuff(ReadWriteLock* lock, int* value) {
    for (int i = 0; i < 40; i++) {
      AutoWriteLock locker(*lock);
      int v = *value;
      ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 10));
      *value = v + 1;
    }
  }

  int Main() override {
    doStuff(lock_, value_);
    return 0;
  }

 private:
  ReadWriteLock* lock_;
  int* value_;

  DISALLOW_COPY_AND_ASSIGN(WriterReadWriteLockTestThread);
};

// Tests that writer locks actually exclude.
TEST(ReadWriteLockTest, MutexTwoThreads) {
  ReadWriteLock lock;
  int value = 0;

  WriterReadWriteLockTestThread thread(&lock, &value);
  thread.Start();

  WriterReadWriteLockTestThread::doStuff(&lock, &value);

  thread.Join();

  EXPECT_EQ(2 * 40, value);
}

TEST(ReadWriteLockTest, MutexFourThreads) {
  ReadWriteLock lock;
  int value = 0;

  WriterReadWriteLockTestThread thread1(&lock, &value);
  WriterReadWriteLockTestThread thread2(&lock, &value);
  WriterReadWriteLockTestThread thread3(&lock, &value);
  thread1.Start();
  thread2.Start();
  thread3.Start();

  WriterReadWriteLockTestThread::doStuff(&lock, &value);

  thread1.Join();
  thread2.Join();
  thread3.Join();

  EXPECT_EQ(4 * 40, value);
}

} // namespace stp
