// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/Alignment.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(AlignmentTest, whichAlignment32) {
  EXPECT_EQ(1u, whichAlignment(0u));
  EXPECT_EQ(1u, whichAlignment(1u));
  EXPECT_EQ(2u, whichAlignment(2u));
  EXPECT_EQ(1u, whichAlignment(3u));
  EXPECT_EQ(4u, whichAlignment(4u));
  EXPECT_EQ(1u, whichAlignment(7u));
  EXPECT_EQ(8u, whichAlignment(8u));
  EXPECT_EQ(2u, whichAlignment(10u));
  EXPECT_EQ(16u, whichAlignment(16u));
  EXPECT_EQ(1u, whichAlignment(1023u));
  EXPECT_EQ(1024u, whichAlignment(1024u));
  EXPECT_EQ(1u, whichAlignment(1025u));
  EXPECT_EQ(512u, whichAlignment(0xEE00u));
}

TEST(AlignmentTest, alignPointer) {
  const unsigned N = 20;
  char buf[N];
  bool r;
  void* p = &buf[0];
  unsigned s = N;
  r = alignForward(p, 4u, 10u, s);
  EXPECT_EQ(p, &buf[0]);
  EXPECT_TRUE(r);
  EXPECT_EQ(s, N);

  p = &buf[1];
  s = N;
  r = alignForward(p, 4u, 10u, s);
  EXPECT_EQ(p, &buf[4]);
  EXPECT_TRUE(r);
  EXPECT_EQ(s, N-3);

  p = &buf[2];
  s = N;
  r = alignForward(p, 4u, 10u, s);
  EXPECT_EQ(p, &buf[4]);
  EXPECT_TRUE(r);
  EXPECT_EQ(s, N-2);

  p = &buf[3];
  s = N;
  r = alignForward(p, 4u, 10u, s);
  EXPECT_EQ(p, &buf[4]);
  EXPECT_TRUE(r);
  EXPECT_EQ(s, N-1);

  p = &buf[4];
  s = N;
  r = alignForward(p, 4u, 10u, s);
  EXPECT_EQ(p, &buf[4]);
  EXPECT_TRUE(r);
  EXPECT_EQ(s, N);

  p = &buf[0];
  s = N;
  r = alignForward(p, 4u, N, s);
  EXPECT_EQ(p, &buf[0]);
  EXPECT_TRUE(r);
  EXPECT_EQ(s, N);

  p = &buf[1];
  s = N-1;
  r = alignForward(p, 4u, N-4, s);
  EXPECT_EQ(p, &buf[4]);
  EXPECT_TRUE(r);
  EXPECT_EQ(s, N-4);

  p = &buf[1];
  s = N-1;
  r = alignForward(p, 4u, N-3, s);
  EXPECT_EQ(p, &buf[1]);
  EXPECT_FALSE(r);
  EXPECT_EQ(s, N-1);

  p = &buf[0];
  s = N;
  r = alignForward(p, 1u, N+1, s);
  EXPECT_EQ(p, &buf[0]);
  EXPECT_FALSE(r);
  EXPECT_EQ(s, N);
}

} // namespace stp
