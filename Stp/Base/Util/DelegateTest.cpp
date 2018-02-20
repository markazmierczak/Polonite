// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/Delegate.h"

#include "Base/Test/GMock.h"
#include "Base/Test/GTest.h"

namespace stp {

struct DelegateTest : public testing::Test {
  virtual int callbackWithResult(int a, int b) {
    return a - b;
  }
  int nonVirtualCallbackWithResult(int a, int b) {
    return a - b;
  }
  virtual MOCK_METHOD2(callback, void(int, int));
  MOCK_METHOD2(nonVirtualCallback, void(int, int));
};

TEST_F(DelegateTest, Result) {
  Delegate<int(int, int)> v;
  v = makeDelegate(&DelegateTest::callbackWithResult, this);
  EXPECT_EQ(3, v(5, 2));
  v = makeDelegate(&DelegateTest::nonVirtualCallbackWithResult, this);
  EXPECT_EQ(3, v(5, 2));
}

TEST_F(DelegateTest, NoResult) {
  using testing::Eq;

  Delegate<void(int, int)> v;
  {
    EXPECT_CALL(*this, callback(Eq(5), Eq(2)));
    v = makeDelegate(&DelegateTest::callback, this);
    v(5, 2);
  }
  {
    EXPECT_CALL(*this, nonVirtualCallback(Eq(3), Eq(4))).Times(2);
    v = makeDelegate(&DelegateTest::nonVirtualCallback, this);
    v(3, 4);
    v(3, 4);
  }
}

} // namespace stp
