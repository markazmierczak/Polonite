// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/App/AtExit.h"

#include "Base/Test/GTest.h"

namespace stp {
namespace {

int g_test_counter1 = 0;
int g_test_counter2 = 0;

void incrementTestCounter1(void* unused) {
  ++g_test_counter1;
}

void incrementTestCounter2(void* unused) {
  ++g_test_counter2;
}

void zeroTestCounters() {
  g_test_counter1 = 0;
  g_test_counter2 = 0;
}

void expectCounter1IsZero(void* unused) {
  EXPECT_EQ(0, g_test_counter1);
}

void expectParamIsnullptr(void* param) {
  EXPECT_EQ(static_cast<void*>(nullptr), param);
}

void expectParamIsCounter(void* param) {
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
  zeroTestCounters();
  AtExitManager::registerCallback(&incrementTestCounter1, nullptr);
  AtExitManager::registerCallback(&incrementTestCounter2, nullptr);
  AtExitManager::registerCallback(&incrementTestCounter1, nullptr);

  EXPECT_EQ(0, g_test_counter1);
  EXPECT_EQ(0, g_test_counter2);
  AtExitManager::processCallbacksNow();
  EXPECT_EQ(2, g_test_counter1);
  EXPECT_EQ(1, g_test_counter2);
}

TEST_F(AtExitTest, LIFOOrder) {
  zeroTestCounters();
  AtExitManager::registerCallback(&incrementTestCounter1, nullptr);
  AtExitManager::registerCallback(&expectCounter1IsZero, nullptr);
  AtExitManager::registerCallback(&incrementTestCounter2, nullptr);

  EXPECT_EQ(0, g_test_counter1);
  EXPECT_EQ(0, g_test_counter2);
  AtExitManager::processCallbacksNow();
  EXPECT_EQ(1, g_test_counter1);
  EXPECT_EQ(1, g_test_counter2);
}

TEST_F(AtExitTest, Param) {
  AtExitManager::registerCallback(&expectParamIsnullptr, nullptr);
  AtExitManager::registerCallback(&expectParamIsCounter, &g_test_counter1);
  AtExitManager::processCallbacksNow();
}

TEST_F(AtExitTest, Task) {
  zeroTestCounters();
  AtExitManager::registerCallback(&expectParamIsCounter, &g_test_counter1);
  AtExitManager::processCallbacksNow();
}

} // namespace stp
