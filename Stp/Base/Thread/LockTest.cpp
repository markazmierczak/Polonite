// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/Lock.h"

#include "Base/Test/GTest.h"
#include "Base/Thread/Thread.h"

#include <stdlib.h>

namespace stp {

class BasicLockTestThread : public Thread {
 public:
  explicit BasicLockTestThread(Lock* lock) : lock_(*lock), acquired_(0) {}

  int Main() override {
    for (int i = 0; i < 10; i++) {
      lock_.acquire();
      acquired_++;
      lock_.release();
    }
    for (int i = 0; i < 10; i++) {
      lock_.acquire();
      acquired_++;
      ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
      lock_.release();
    }
    for (int i = 0; i < 10; i++) {
      if (lock_.tryAcquire()) {
        acquired_++;
        ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
        lock_.release();
      }
    }
    return 0;
  }

  int getAcquired() const { return acquired_; }

 private:
  Lock& lock_;
  int acquired_;
};

// Basic test to make sure that acquire()/release()/tryAcquire() don't crash
TEST(LockTest, Basic) {
  Lock lock;
  BasicLockTestThread thread(&lock);
  thread.Start();

  int acquired = 0;
  for (int i = 0; i < 5; i++) {
    lock.acquire();
    acquired++;
    lock.release();
  }
  for (int i = 0; i < 10; i++) {
    lock.acquire();
    acquired++;
    ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
    lock.release();
  }
  for (int i = 0; i < 10; i++) {
    if (lock.tryAcquire()) {
      acquired++;
      ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
      lock.release();
    }
  }
  for (int i = 0; i < 5; i++) {
    lock.acquire();
    acquired++;
    ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
    lock.release();
  }

  thread.Join();

  EXPECT_GE(acquired, 20);
  EXPECT_GE(thread.getAcquired(), 20);
}

class TryLockTestThread : public Thread {
 public:
  explicit TryLockTestThread(Lock* lock) : lock_(*lock), got_lock_(false) {}

  int Main() override {
    got_lock_ = lock_.tryAcquire();
    if (got_lock_)
      lock_.release();
    return 0;
  }

  bool gotLock() const { return got_lock_; }

 private:
  Lock& lock_;
  bool got_lock_;

  DISALLOW_COPY_AND_ASSIGN(TryLockTestThread);
};

// Test that Try() works as expected
TEST(LockTest, TryLock) {
  Lock lock;

  ASSERT_TRUE(lock.tryAcquire());
  // We now have the lock....

  // This thread will not be able to get the lock.
  {
    TryLockTestThread thread(&lock);
    thread.Start();
    thread.Join();
    ASSERT_FALSE(thread.gotLock());
  }

  lock.release();

  // This thread will....
  {
    TryLockTestThread thread(&lock);
    thread.Start();
    thread.Join();

    ASSERT_TRUE(thread.gotLock());
    // But it released it....
    ASSERT_TRUE(lock.tryAcquire());
  }

  lock.release();
}

class MutexLockTestThread : public Thread {
 public:
  MutexLockTestThread(Lock* lock, int* value) : lock_(lock), value_(value) {}

  // Static helper which can also be called from the main thread.
  static void doStuff(Lock* lock, int* value) {
    for (int i = 0; i < 40; i++) {
      lock->acquire();
      int v = *value;
      ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 10));
      *value = v + 1;
      lock->release();
    }
  }

  int Main() override {
    doStuff(lock_, value_);
    return 0;
  }

 private:
  Lock* lock_;
  int* value_;

  DISALLOW_COPY_AND_ASSIGN(MutexLockTestThread);
};

// Tests that locks actually exclude
TEST(LockTest, MutexTwoThreads) {
  Lock lock;
  int value = 0;

  MutexLockTestThread thread(&lock, &value);
  thread.Start();
  MutexLockTestThread::doStuff(&lock, &value);
  thread.Join();
  EXPECT_EQ(2 * 40, value);
}

TEST(LockTest, MutexFourThreads) {
  Lock lock;
  int value = 0;

  MutexLockTestThread thread1(&lock, &value);
  MutexLockTestThread thread2(&lock, &value);
  MutexLockTestThread thread3(&lock, &value);
  thread1.Start();
  thread2.Start();
  thread3.Start();

  MutexLockTestThread::doStuff(&lock, &value);

  thread1.Join();
  thread2.Join();
  thread3.Join();

  EXPECT_EQ(4 * 40, value);
}

} // namespace stp
