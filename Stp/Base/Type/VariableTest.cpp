// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/Variable.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(VariableTest, FloatIntFloat) {
  float f = 3.1415926f;
  int i = bit_cast<int32_t>(f);
  float f2 = bit_cast<float>(i);
  EXPECT_EQ(f, f2);
}

TEST(VariableTest, StructureInt) {
  struct A { int x; };

  A a = { 1 };
  int b = bit_cast<int>(a);
  EXPECT_EQ(1, b);
}

} // namespace stp
