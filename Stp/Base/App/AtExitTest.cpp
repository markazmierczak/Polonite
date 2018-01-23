// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/App/AtExit.h"

#include "Base/Test/GTest.h"

namespace stp {
namespace {

int g_test_counter1 = 0;
int g_test_counter2 = 0;

void IncrementTestCounter1(void* unused) {
  ++g_test_counter1;
}

void IncrementTestCounter2(void* unused) {
  ++g_test_counter2;
}

void ZeroTestCounters() {
  g_test_counter1 = 0;
  g_test_counter2 = 0;
}

void ExpectCounter1IsZero(void* unused) {
  EXPECT_EQ(0, g_test_counter1);
}

void ExpectParamIsnullptr(void* param) {
  EXPECT_EQ(static_cast<void*>(nullptr), param);
}

void ExpectParamIsCounter(void* param) {
  EXPECT_EQ(&g_test_counter1, param);
}

} // namespace

class AtExitTest : public testing::Test {
 private:
  // Don't test the global AtExitManager, because asking it to process its
  // AtExit callbacks can ruin the global state that other tests may depend on.
  ShadowingAtExitManager exit_manager_;
};

TEST_F(AtExitTest, Basic) {
  ZeroTestCounters();
  AtExitManager::RegisterCallback(&IncrementTestCounter1, nullptr);
  AtExitManager::RegisterCallback(&IncrementTestCounter2, nullptr);
  AtExitManager::RegisterCallback(&IncrementTestCounter1, nullptr);

  EXPECT_EQ(0, g_test_counter1);
  EXPECT_EQ(0, g_test_counter2);
  AtExitManager::ProcessCallbacksNow();
  EXPECT_EQ(2, g_test_counter1);
  EXPECT_EQ(1, g_test_counter2);
}

TEST_F(AtExitTest, LIFOOrder) {
  ZeroTestCounters();
  AtExitManager::RegisterCallback(&IncrementTestCounter1, nullptr);
  AtExitManager::RegisterCallback(&ExpectCounter1IsZero, nullptr);
  AtExitManager::RegisterCallback(&IncrementTestCounter2, nullptr);

  EXPECT_EQ(0, g_test_counter1);
  EXPECT_EQ(0, g_test_counter2);
  AtExitManager::ProcessCallbacksNow();
  EXPECT_EQ(1, g_test_counter1);
  EXPECT_EQ(1, g_test_counter2);
}

TEST_F(AtExitTest, Param) {
  AtExitManager::RegisterCallback(&ExpectParamIsnullptr, nullptr);
  AtExitManager::RegisterCallback(&ExpectParamIsCounter, &g_test_counter1);
  AtExitManager::ProcessCallbacksNow();
}

TEST_F(AtExitTest, Task) {
  ZeroTestCounters();
  AtExitManager::RegisterCallback(&ExpectParamIsCounter, &g_test_counter1);
  AtExitManager::ProcessCallbacksNow();
}

} // namespace stp
