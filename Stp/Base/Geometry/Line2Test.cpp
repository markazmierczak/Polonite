// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Geometry/Line2.h"

#include "Base/Geometry/Bounds2.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(LineTest, Intersects) {
  const float L = 0;
  const float T = 0;
  const float R = 100;
  const float B = 100;
  const float CX = (L + R) / 2;
  const float CY = (T + B) / 2;

  const auto RV = Bounds2(0, 0, R, B);

  Line2 dst;

  const Line2 Empty[] = {
    // sides
    { L, CY, L - 10, CY },
    { R, CY, R + 10, CY },
    { CX, T, CX, T - 10 },
    { CX, B, CX, B + 10 },
    // corners
    { L, T, L - 10, T - 10 },
    { L, B, L - 10, B + 10 },
    { R, T, R + 10, T - 10 },
    { R, B, R + 10, B + 10 },
  };
  for (const auto& line : Empty)
    EXPECT_FALSE(Line2::Intersects(line, RV));

  const Line2 Full[] = {
    // diagonals, chords
    { L, T, R, B },
    { L, B, R, T },
    { CX, T, CX, B },
    { L, CY, R, CY },
    { CX, T, R, CY },
    { CX, T, L, CY },
    { L, CY, CX, B },
    { R, CY, CX, B },
    // edges
    { L, T, L, B },
    { R, T, R, B },
    { L, T, R, T },
    { L, B, R, B },
  };
  for (const auto& line : Full) {
    EXPECT_TRUE(Line2::Intersects(line, RV, &dst));
    EXPECT_EQ(line, dst);
  }

  const Line2 Partial[] = {
    { L - 10, CY, CX, CY }, { L, CY, CX, CY },
    { CX, T - 10, CX, CY }, { CX, T, CX, CY },
    { R + 10, CY, CX, CY }, { R, CY, CX, CY },
    { CX, B + 10, CX, CY }, { CX, B, CX, CY },
    // extended edges
    { L, T - 10, L, B + 10 }, { L, T, L, B },
    { R, T - 10, R, B + 10 }, { R, T, R, B },
    { L - 10, T, R + 10, T }, { L, T, R, T },
    { L - 10, B, R + 10, B }, { L, B, R, B },
  };
  for (int i = 0; i < ArraySizeOf(Partial); i += 2) {
    EXPECT_TRUE(Line2::Intersects(Partial[i], RV, &dst));
    EXPECT_EQ(Partial[i + 1], dst);
  }
}

} // namespace stp
