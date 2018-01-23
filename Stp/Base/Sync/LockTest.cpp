// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Sync/Lock.h"

#include "Base/Test/GTest.h"
#include "Base/Thread/Thread.h"

#include <stdlib.h>

namespace stp {

// Basic test to make sure that Acquire()/Release()/Try() don't crash ----------

class BasicLockTestThread : public Thread {
 public:
  explicit BasicLockTestThread(Lock* lock) : lock_(lock), acquired_(0) {}

  int Main() override {
    for (int i = 0; i < 10; i++) {
      lock_->Acquire();
      acquired_++;
      lock_->Release();
    }
    for (int i = 0; i < 10; i++) {
      lock_->Acquire();
      acquired_++;
      ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
      lock_->Release();
    }
    for (int i = 0; i < 10; i++) {
      if (lock_->TryAcquire()) {
        acquired_++;
        ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
        lock_->Release();
      }
    }
    return 0;
  }

  int acquired() const { return acquired_; }

 private:
  Lock* lock_;
  int acquired_;

  DISALLOW_COPY_AND_ASSIGN(BasicLockTestThread);
};

TEST(LockTest, Basic) {
  Lock lock;
  BasicLockTestThread thread(&lock);
  thread.Start();

  int acquired = 0;
  for (int i = 0; i < 5; i++) {
    lock.Acquire();
    acquired++;
    lock.Release();
  }
  for (int i = 0; i < 10; i++) {
    lock.Acquire();
    acquired++;
    ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
    lock.Release();
  }
  for (int i = 0; i < 10; i++) {
    if (lock.TryAcquire()) {
      acquired++;
      ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
      lock.Release();
    }
  }
  for (int i = 0; i < 5; i++) {
    lock.Acquire();
    acquired++;
    ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 20));
    lock.Release();
  }

  thread.Join();

  EXPECT_GE(acquired, 20);
  EXPECT_GE(thread.acquired(), 20);
}

// Test that Try() works as expected -------------------------------------------

class TryLockTestThread : public Thread {
 public:
  explicit TryLockTestThread(Lock* lock) : lock_(lock), got_lock_(false) {}

  int Main() override {
    got_lock_ = lock_->TryAcquire();
    if (got_lock_)
      lock_->Release();
    return 0;
  }

  bool got_lock() const { return got_lock_; }

 private:
  Lock* lock_;
  bool got_lock_;

  DISALLOW_COPY_AND_ASSIGN(TryLockTestThread);
};

TEST(LockTest, TryLock) {
  Lock lock;

  ASSERT_TRUE(lock.TryAcquire());
  // We now have the lock....

  // This thread will not be able to get the lock.
  {
    TryLockTestThread thread(&lock);
    thread.Start();
    thread.Join();
    ASSERT_FALSE(thread.got_lock());
  }

  lock.Release();

  // This thread will....
  {
    TryLockTestThread thread(&lock);
    thread.Start();
    thread.Join();

    ASSERT_TRUE(thread.got_lock());
    // But it released it....
    ASSERT_TRUE(lock.TryAcquire());
  }

  lock.Release();
}

// Tests that locks actually exclude -------------------------------------------

class MutexLockTestThread : public Thread {
 public:
  MutexLockTestThread(Lock* lock, int* value) : lock_(lock), value_(value) {}

  // Static helper which can also be called from the main thread.
  static void DoStuff(Lock* lock, int* value) {
    for (int i = 0; i < 40; i++) {
      lock->Acquire();
      int v = *value;
      ThisThread::SleepFor(TimeDelta::FromMilliseconds(rand() % 10));
      *value = v + 1;
      lock->Release();
    }
  }

  int Main() override {
    DoStuff(lock_, value_);
    return 0;
  }

 private:
  Lock* lock_;
  int* value_;

  DISALLOW_COPY_AND_ASSIGN(MutexLockTestThread);
};

TEST(LockTest, MutexTwoThreads) {
  Lock lock;
  int value = 0;

  MutexLockTestThread thread(&lock, &value);
  thread.Start();
  MutexLockTestThread::DoStuff(&lock, &value);
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

  MutexLockTestThread::DoStuff(&lock, &value);

  thread1.Join();
  thread2.Join();
  thread3.Join();

  EXPECT_EQ(4 * 40, value);
}

} // namespace stp
