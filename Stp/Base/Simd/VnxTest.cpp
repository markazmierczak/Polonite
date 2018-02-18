// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2015 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Simd/Vnx.h"

#include "Base/Compiler/Sanitizer.h"
#include "Base/Util/Random.h"
#include "Base/Test/GTest.h"

namespace stp {

template<int N>
static void TestTemplateVecNf() {
  auto assert_nearly_eq = [&](float eps, const VecNx<N, float>& v,
      float a, float b, float c, float d) {
    auto close = [=](float a, float b) { return fabsf(a-b) <= eps; };
    float vals[4];
    v.Store(vals);
    bool ok = close(vals[0], a) && close(vals[1], b) &&
              close(   v[0], a) && close(   v[1], b);
    EXPECT_TRUE(ok);
    if (N == 4) {
      ok = close(vals[2], c) && close(vals[3], d) &&
           close(   v[2], c) && close(   v[3], d);
      EXPECT_TRUE(ok);
    }
  };
  auto assert_eq = [&](const VecNx<N, float>& v, float a, float b, float c, float d) {
    return assert_nearly_eq(0, v, a,b,c,d);
  };

  float vals[] = {3, 4, 5, 6};
  VecNx<N,float> a = VecNx<N,float>::Load(vals),
      b(a),
      c = a;
  VecNx<N,float> d;
  d = a;

  assert_eq(a, 3, 4, 5, 6);
  assert_eq(b, 3, 4, 5, 6);
  assert_eq(c, 3, 4, 5, 6);
  assert_eq(d, 3, 4, 5, 6);

  assert_eq(a+b, 6, 8, 10, 12);
  assert_eq(a*b, 9, 16, 25, 36);
  assert_eq(a*b-b, 6, 12, 20, 30);
  assert_eq((a*b).Sqrt(), 3, 4, 5, 6);
  assert_eq(a/b, 1, 1, 1, 1);
  assert_eq(VecNx<N,float>(0)-a, -3, -4, -5, -6);

  VecNx<N,float> fours(4);

  assert_eq(fours.Sqrt(), 2,2,2,2);
  assert_nearly_eq(0.001f, fours.RSqrt(), 0.5, 0.5, 0.5, 0.5);

  assert_nearly_eq(0.001f, fours.Reciprocal(), 0.25, 0.25, 0.25, 0.25);

  assert_eq(min(a, fours), 3, 4, 4, 4);
  assert_eq(max(a, fours), 4, 4, 5, 6);

  // Test some comparisons.  This is not exhaustive.
  EXPECT_TRUE((a == b).AllTrue());
  EXPECT_TRUE((a+b == a*b-b).AnyTrue());
  EXPECT_FALSE((a+b == a*b-b).AllTrue());
  EXPECT_FALSE((a+b == a*b).AnyTrue());
  EXPECT_FALSE((a != b).AnyTrue());
  EXPECT_TRUE((a < fours).AnyTrue());
  EXPECT_TRUE((a <= fours).AnyTrue());
  EXPECT_FALSE((a > fours).AllTrue());
  EXPECT_FALSE((a >= fours).AllTrue());
}

TEST(VnxTest, Vecf) {
  TestTemplateVecNf<2>();
  TestTemplateVecNf<4>();
}

template<int N, typename T>
void test_Ni() {
  auto assert_eq = [&](const VecNx<N,T>& v, T a, T b, T c, T d, T e, T f, T g, T h) {
    T vals[8];
    v.Store(vals);

    switch (N) {
      case 8: EXPECT_TRUE(vals[4] == e && vals[5] == f && vals[6] == g && vals[7] == h);
      case 4: EXPECT_TRUE(vals[2] == c && vals[3] == d);
      case 2: EXPECT_TRUE(vals[0] == a && vals[1] == b);
    }
    switch (N) {
      case 8: EXPECT_TRUE(v[4] == e && v[5] == f &&
                              v[6] == g && v[7] == h);
      case 4: EXPECT_TRUE(v[2] == c && v[3] == d);
      case 2: EXPECT_TRUE(v[0] == a && v[1] == b);
    }
  };

  T vals[] = { 1,2,3,4,5,6,7,8 };
  VecNx<N,T> a = VecNx<N,T>::Load(vals),
      b(a),
      c = a;
  VecNx<N,T> d;
  d = a;

  assert_eq(a, 1,2,3,4,5,6,7,8);
  assert_eq(b, 1,2,3,4,5,6,7,8);
  assert_eq(c, 1,2,3,4,5,6,7,8);
  assert_eq(d, 1,2,3,4,5,6,7,8);

  assert_eq(a+a, 2,4,6,8,10,12,14,16);
  assert_eq(a*a, 1,4,9,16,25,36,49,64);
  assert_eq(a*a-a, 0,2,6,12,20,30,42,56);

  assert_eq(a >> 2, 0,0,0,1,1,1,1,2);
  assert_eq(a << 1, 2,4,6,8,10,12,14,16);

  EXPECT_TRUE(a[1] == 2);
}

TEST(VnxTest, Veci) {
  test_Ni<2, uint16_t>();
  test_Ni<4, uint16_t>();
  test_Ni<8, uint16_t>();

  test_Ni<2, int32_t>();
  test_Ni<4, int32_t>();
  test_Ni<8, int32_t>();
}

TEST(VnxTest, MinLt) {
  // Exhaustively check the 8x8 bit space.
  for (int a = 0; a < (1<<8); a++) {
    for (int b = 0; b < (1<<8); b++) {
      Vec16b aw(a), bw(b);
      EXPECT_EQ(min(a, b), min(aw, bw)[0]);
      EXPECT_EQ(!(a < b), !(aw < bw)[0]);
    }
  }
}

// Exhausting the 16x16 bit space is slow, so only do that manually.
#if 0
TEST(VnxTest, MinLtExhaustive) {
  for (int a = 0; a < (1<<16); a++) {
    for (int b = 0; b < (1<<16); b++)
      EXPECT_EQ(min(a, b), min(Vec8h(a), Vec8h(b))[0]);
  }
}
#endif

TEST(VnxTest, SaturatedAdd) {
  for (int a = 0; a < (1<<8); a++) {
    for (int b = 0; b < (1<<8); b++) {
      int exact = a + b;
      if (exact > 255) { exact = 255; }
      if (exact <   0) { exact =   0; }

      EXPECT_EQ(exact, VnxMath::SaturatedAdd(Vec16b(a), Vec16b(b))[0]);
    }
  }
}

TEST(VnxTest, Abs) {
  Vec4f fs = Abs(Vec4f(0.f, -0.f, 2.f, -4.f));
  EXPECT_EQ(0.f, fs[0]);
  EXPECT_EQ(0.f, fs[1]);
  EXPECT_EQ(2.f, fs[2]);
  EXPECT_EQ(4.f, fs[3]);
}

TEST(VnxTest, Floor) {
  Vec4f fs = Vec4f(0.4f, -0.4f, 0.6f, -0.6f).Floor();
  EXPECT_EQ( 0.f, fs[0]);
  EXPECT_EQ(-1.f, fs[1]);
  EXPECT_EQ( 0.f, fs[2]);
  EXPECT_EQ(-1.f, fs[3]);
}

TEST(VnxTest, Shuffle) {
  Vec4f f4(0, 10, 20, 30);

  Vec2f f2 = VnxMath::Shuffle<2,1>(f4);
  EXPECT_EQ(20, f2[0]);
  EXPECT_EQ(10, f2[1]);

  f4 = VnxMath::Shuffle<0,1,1,0>(f2);
  EXPECT_EQ(20, f4[0]);
  EXPECT_EQ(10, f4[1]);
  EXPECT_EQ(10, f4[2]);
  EXPECT_EQ(20, f4[3]);
}

TEST(VnxTest, IntFloatConversion) {
  Vec4f f(-2.3f, 1.f, 0.45f, 0.6f);

  Vec4i i = vnx_cast<int>(f);
  EXPECT_EQ(-2, i[0]);
  EXPECT_EQ( 1, i[1]);
  EXPECT_EQ( 0, i[2]);
  EXPECT_EQ( 0, i[3]);

  f = vnx_cast<float>(i);
  EXPECT_EQ(-2.f, f[0]);
  EXPECT_EQ( 1.f, f[1]);
  EXPECT_EQ( 0.f, f[2]);
  EXPECT_EQ( 0.f, f[3]);
}

TEST(VnxTest, UInt16FloatConversion) {
  {
    // u16 --> float
    Vec4h h4 = Vec4h(15, 17, 257, 65535);
    Vec4f f4 = vnx_cast<float>(h4);
    EXPECT_EQ(   15.f, f4[0]);
    EXPECT_EQ(   17.f, f4[1]);
    EXPECT_EQ(  257.f, f4[2]);
    EXPECT_EQ(65535.f, f4[3]);
  }
  {
    // float -> u16
    Vec4f f4 = Vec4f(15, 17, 257, 65535);
    Vec4h h4 = vnx_cast<uint16_t>(f4);
    EXPECT_EQ(   15u, h4[0]);
    EXPECT_EQ(   17u, h4[1]);
    EXPECT_EQ(  257u, h4[2]);
    EXPECT_EQ(65535u, h4[3]);
  }

  // starting with any u16 value, we should be able to have a perfect round-trip
  // in/out of floats
  Random rand;
  auto NextUInt16 = [&rand]() { return static_cast<uint16_t>(rand.NextUInt32()); };
  for (int i = 0; i < 10000; ++i) {
    const uint16_t s16[4] {
      NextUInt16(), NextUInt16(),
      NextUInt16(), NextUInt16(),
    };
    Vec4h u4_0 = Vec4h::Load(s16);
    Vec4f f4 = vnx_cast<float>(u4_0);
    Vec4h u4_1 = vnx_cast<uint16_t>(f4);
    uint16_t d16[4];
    u4_1.Store(d16);
    EXPECT_TRUE(memcmp(s16, d16, sizeof(s16)) == 0);
  }
}

// The SSE2 implementation of Cast<uint16_t>(Vec4i) is non-trivial, so worth a test.
TEST(VnxTest, Int32UInt16Conversion) {
  // These are pretty hard to get wrong.
  for (int i = 0; i <= 0x7FFF; i++) {
    uint16_t expected = (uint16_t)i;
    uint16_t actual = vnx_cast<uint16_t>(Vec4i(i))[0];

    EXPECT_EQ(expected, actual);
  }

  // A naive implementation with _mm_packs_epi32 would succeed up to 0x7fff but fail here:
  for (int i = 0x8000; (1) && i <= 0xFFFF; i++) {
    uint16_t expected = (uint16_t)i;
    uint16_t actual = vnx_cast<uint16_t>(Vec4i(i))[0];

    EXPECT_EQ(expected, actual);
  }
}

} // namespace stp
