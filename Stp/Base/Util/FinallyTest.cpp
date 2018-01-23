// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/Finally.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(FinallyTest, Basic) {
  bool a = false;
  bool b = false;
  bool c = false;
  bool d = false;
  bool e = false;

  auto lam = [&]{ c = true; };
  try {
    auto v = MakeScopeFinally([&]{ a = true; });
    auto w = MakeScopeContinue([&]{ b = true; });
    auto x = MakeScopeCatch(lam);
    throw 42;
  } catch(...) {
    auto y = MakeScopeCatch([&]{ d = true; });
    auto z = MakeScopeContinue([&]{ e = true; });
  }
  EXPECT_TRUE(a);
  EXPECT_FALSE(b);
  EXPECT_TRUE(c);
  EXPECT_FALSE(d);
  EXPECT_TRUE(e);
}

} // namespace stp
