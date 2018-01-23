// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Sync/WaitableEvent.h"

#include "Base/Test/GTest.h"
#include "Base/Thread/Thread.h"
#include "Base/Time/TimeTicks.h"

namespace stp {

TEST(WaitableEventTest, ManualBasics) {
  WaitableEvent event(WaitableEvent::ResetPolicy::Manual,
                      WaitableEvent::InitialState::NotSignaled);

  EXPECT_FALSE(event.IsSignaled());

  event.Signal();
  EXPECT_TRUE(event.IsSignaled());
  EXPECT_TRUE(event.IsSignaled());

  event.Reset();
  EXPECT_FALSE(event.IsSignaled());
  EXPECT_FALSE(event.TimedWait(TimeDelta::FromMilliseconds(10)));

  event.Signal();
  event.Wait();
  EXPECT_TRUE(event.TimedWait(TimeDelta::FromMilliseconds(10)));
}

TEST(WaitableEventTest, AutoBasics) {
  WaitableEvent event(WaitableEvent::ResetPolicy::Automatic,
                      WaitableEvent::InitialState::NotSignaled);

  EXPECT_FALSE(event.IsSignaled());

  event.Signal();
  EXPECT_TRUE(event.IsSignaled());
  EXPECT_FALSE(event.IsSignaled());

  event.Reset();
  EXPECT_FALSE(event.IsSignaled());
  EXPECT_FALSE(event.TimedWait(TimeDelta::FromMilliseconds(10)));

  event.Signal();
  event.Wait();
  EXPECT_FALSE(event.TimedWait(TimeDelta::FromMilliseconds(10)));

  event.Signal();
  EXPECT_TRUE(event.TimedWait(TimeDelta::FromMilliseconds(10)));
}

TEST(WaitableEventTest, WaitManyShortcut) {
  WaitableEvent* ev[5];
  for (unsigned i = 0; i < 5; ++i) {
    ev[i] = new WaitableEvent(WaitableEvent::ResetPolicy::Automatic,
                              WaitableEvent::InitialState::NotSignaled);
  }

  ev[3]->Signal();
  EXPECT_EQ(3, WaitableEvent::WaitMany(ev, 5));

  ev[3]->Signal();
  EXPECT_EQ(3, WaitableEvent::WaitMany(ev, 5));

  ev[4]->Signal();
  EXPECT_EQ(4, WaitableEvent::WaitMany(ev, 5));

  ev[0]->Signal();
  EXPECT_EQ(0, WaitableEvent::WaitMany(ev, 5));

  for (unsigned i = 0; i < 5; ++i)
    delete ev[i];
}

class WaitableEventSignaler : public Thread {
 public:
  WaitableEventSignaler(TimeDelta delay, WaitableEvent* event)
      : delay_(delay),
        event_(event) {
  }

  int Main() override {
    ThisThread::SleepFor(delay_);
    event_->Signal();
    return 0;
  }

 private:
  const TimeDelta delay_;
  WaitableEvent* event_;
};

// Tests that a WaitableEvent can be safely deleted when |Wait| is done without
// additional synchronization.
TEST(WaitableEventTest, WaitAndDelete) {
  WaitableEvent* ev =
      new WaitableEvent(WaitableEvent::ResetPolicy::Automatic,
                        WaitableEvent::InitialState::NotSignaled);

  WaitableEventSignaler signaler(TimeDelta::FromMilliseconds(10), ev);
  signaler.Start();

  ev->Wait();
  delete ev;

  signaler.Join();
}

// Tests that a WaitableEvent can be safely deleted when |WaitMany| is done
// without additional synchronization.
TEST(WaitableEventTest, WaitMany) {
  WaitableEvent* ev[5];
  for (unsigned i = 0; i < 5; ++i) {
    ev[i] = new WaitableEvent(WaitableEvent::ResetPolicy::Automatic,
                              WaitableEvent::InitialState::NotSignaled);
  }

  WaitableEventSignaler signaler(TimeDelta::FromMilliseconds(10), ev[2]);
  signaler.Start();

  int index = WaitableEvent::WaitMany(ev, 5);

  for (int i = 0; i < 5; ++i)
    delete ev[i];

  signaler.Join();
  EXPECT_EQ(2, index);
}

// Tests that a sub-ms TimedWait doesn't time out promptly.
TEST(WaitableEventTest, SubMsTimedWait) {
  WaitableEvent ev(WaitableEvent::ResetPolicy::Automatic,
                   WaitableEvent::InitialState::NotSignaled);

  TimeDelta delay = TimeDelta::FromMicroseconds(900);
  TimeTicks start_time = TimeTicks::Now();
  ev.TimedWait(delay);
  EXPECT_GE(TimeTicks::Now() - start_time, delay);
}

// Tests that TimedWaitUntil can be safely used with various end_time deadline
// values.
TEST(WaitableEventTest, TimedWaitUntil) {
  WaitableEvent ev(WaitableEvent::ResetPolicy::Automatic,
                   WaitableEvent::InitialState::NotSignaled);

  TimeTicks start_time(TimeTicks::Now());
  TimeDelta delay = TimeDelta::FromMilliseconds(10);

  // Should be OK to wait for the current time or time in the past.
  // That should end promptly and be equivalent to IsSignalled.
  EXPECT_FALSE(ev.TimedWaitUntil(start_time));
  EXPECT_FALSE(ev.TimedWaitUntil(start_time - delay));

  // Should be OK to wait for zero TimeTicks().
  EXPECT_FALSE(ev.TimedWaitUntil(TimeTicks()));

  // Waiting for a time in the future shouldn't end before the deadline
  // if the event isn't signalled.
  EXPECT_FALSE(ev.TimedWaitUntil(start_time + delay));
  EXPECT_GE(TimeTicks::Now() - start_time, delay);

  // Test that passing TimeTicks::Max to TimedWaitUntil is valid and isn't
  // the same as passing TimeTicks(). Also verifies that signaling event
  // ends the wait promptly.
  WaitableEventSignaler signaler(delay, &ev);
  start_time = TimeTicks::Now();
  signaler.Start();
  signaler.Join();
}

} // namespace stp
