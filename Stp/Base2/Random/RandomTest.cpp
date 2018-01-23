// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Random/Random.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(PseudoRandomTest, Determinism) {
  // Two generators seeded with the same value should produce the same sequence.
  Random a;
  Random b;
  for (int i = 0; i < 4; ++i)
    b.NextUInt32();
  b.Seed(456);
  for (int i = 0; i < 4; ++i)
    b.NextUInt32();
  EXPECT_NE(a.NextUInt32(), b.NextUInt32());
  a.Seed(123);
  b.Seed(123);
  for (int i = 0; i < 100; i++)
    EXPECT_EQ(a.NextUInt32(), b.NextUInt32());
}

} // namespace stp
