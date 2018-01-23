// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Call/Delegate.h"

#include "Base/Test/GMock.h"
#include "Base/Test/GTest.h"

namespace stp {

struct DelegateTest : public testing::Test {
  virtual int CallbackWithResult(int a, int b) {
    return a - b;
  }
  int NonVirtualCallbackWithResult(int a, int b) {
    return a - b;
  }
  virtual MOCK_METHOD2(Callback, void(int, int));
  MOCK_METHOD2(NonVirtualCallback, void(int, int));
};

TEST_F(DelegateTest, Result) {
  Delegate<int(int, int)> v;
  v = MakeDelegate(&DelegateTest::CallbackWithResult, this);
  EXPECT_EQ(3, v(5, 2));
  v = MakeDelegate(&DelegateTest::NonVirtualCallbackWithResult, this);
  EXPECT_EQ(3, v(5, 2));
}

TEST_F(DelegateTest, NoResult) {
  using testing::Eq;

  Delegate<void(int, int)> v;
  {
    EXPECT_CALL(*this, Callback(Eq(5), Eq(2)));
    v = MakeDelegate(&DelegateTest::Callback, this);
    v(5, 2);
  }
  {
    EXPECT_CALL(*this, NonVirtualCallback(Eq(3), Eq(4))).Times(2);
    v = MakeDelegate(&DelegateTest::NonVirtualCallback, this);
    v(3, 4);
    v(3, 4);
  }
}

} // namespace stp
