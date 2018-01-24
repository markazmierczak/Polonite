// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Geometry/Size2.h"

#include "Base/Test/GTest.h"

namespace stp {
namespace {

TEST(IntSize2Test, Clamp) {
  IntSize2 a;

  a = IntSize2(3, 5);
  EXPECT_EQ(IntSize2(3, 5), a);
  a = Max(a, IntSize2(2, 4));
  EXPECT_EQ(IntSize2(3, 5), a);
  a = Max(a, IntSize2(3, 5));
  EXPECT_EQ(IntSize2(3, 5), a);
  a = Max(a, IntSize2(4, 2));
  EXPECT_EQ(IntSize2(4, 5), a);
  a = Max(a, IntSize2(8, 10));
  EXPECT_EQ(IntSize2(8, 10), a);

  a = Min(a, IntSize2(9, 11));
  EXPECT_EQ(IntSize2(8, 10), a);
  a = Min(a, IntSize2(8, 10));
  EXPECT_EQ(IntSize2(8, 10), a);
  a = Min(a, IntSize2(11, 9));
  EXPECT_EQ(IntSize2(8, 9), a);
  a = Min(a, IntSize2(7, 11));
  EXPECT_EQ(IntSize2(7, 9), a);
  a = Min(a, IntSize2(3, 5));
  EXPECT_EQ(IntSize2(3, 5), a);
}

} // namespace
} // namespace stp
