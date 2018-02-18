// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Geometry/Quaternion.h"

#include "Base/Test/GTest.h"

namespace stp {
namespace {

constexpr double TestEpsilon = 1E-7;

void CompareQuaternions(const Quaternion& a, const Quaternion& b) {
  EXPECT_FLOAT_EQ(a.w, b.w);
  EXPECT_FLOAT_EQ(a.x, b.x);
  EXPECT_FLOAT_EQ(a.y, b.y);
  EXPECT_FLOAT_EQ(a.z, b.z);
}

TEST(QuaternionTest, Identity) {
  CompareQuaternions(Quaternion(1, 0, 0, 0), Quaternion::Identity());
  auto ident = Quaternion::Identity();
  EXPECT_TRUE(ident.IsIdentity());
}

TEST(QuaternionTest, AxisAngleCommon) {
  double radians = 0.5;
  auto q = Quaternion::FromAngleAxisUnit(radians, Vector3(1, 0, 0));
  CompareQuaternions(Quaternion(Cos(radians / 2), Sin(radians / 2), 0, 0), q);
}

TEST(QuaternionTest, FromRotationTo) {
  auto q = Quaternion::FromRotationTo(Vector3(1, 0, 0), Vector3(0, 1, 0));
  auto r = Quaternion::FromAngleAxis(Angle::RightInRadians, Vector3(0, 0, 1));

  EXPECT_FLOAT_EQ(r.w, q.w);
  EXPECT_FLOAT_EQ(r.x, q.x);
  EXPECT_FLOAT_EQ(r.y, q.y);
  EXPECT_FLOAT_EQ(r.z, q.z);
}

TEST(QuaternionTest, AxisAngleWithZeroLengthAxis) {
  auto q = Quaternion::FromAngleAxis(0.5, Vector3(0, 0, 0));
  // If the axis of zero length, we should assume the default values.
  CompareQuaternions(q, Quaternion::Identity());
}

TEST(QuaternionTest, Addition) {
  double values[] = {0, 1, 100};
  for (double t : values) {
    Quaternion a(4 * t, t, 2 * t, 3 * t);
    Quaternion b(2 * t, 5 * t, 4 * t, 3 * t);
    Quaternion sum = a + b;
    CompareQuaternions(Quaternion(t, t, t, t) * 6, sum);
  }
}

TEST(QuaternionTest, Multiplication) {
  struct {
    Quaternion a;
    Quaternion b;
    Quaternion expected;
  } cases[] = {
    {Quaternion(0, 1, 0, 0), Quaternion(0, 1, 0, 0), Quaternion(-1, 0, 0, 0)},
    {Quaternion(0, 0, 1, 0), Quaternion(0, 0, 1, 0), Quaternion(-1, 0, 0, 0)},
    {Quaternion(0, 0, 0, 1), Quaternion(0, 0, 0, 1), Quaternion(-1, 0, 0, 0)},
    {Quaternion(1, 0, 0, 0), Quaternion(1, 0, 0, 0), Quaternion(1, 0, 0, 0)},
    {Quaternion(4, 1, 2, 3), Quaternion(8, 5, 6, 7), Quaternion(-6, 24, 48, 48)},
    {Quaternion(8, 5, 6, 7), Quaternion(4, 1, 2, 3), Quaternion(-6, 32, 32, 56)},
  };

  for (const auto& item : cases) {
    Quaternion product = item.a * item.b;
    CompareQuaternions(item.expected, product);
  }
}

TEST(QuaternionTest, Scaling) {
  double values[] = {0, 10, 100};
  for (double s : values) {
    Quaternion q(1, 2, 3, 4);
    Quaternion expected(s, 2 * s, 3 * s, 4 * s);
    CompareQuaternions(expected, q * s);
    CompareQuaternions(expected, s * q);
    if (s > 0)
      CompareQuaternions(expected, q / (1 / s));
  }
}

TEST(QuaternionTest, Lerp) {
  for (int i = 1; i < 100; ++i) {
    Quaternion a(0, 0, 0, 0);
    Quaternion b(1, 2, 3, 4);
    float t = static_cast<float>(i) / 100.0f;
    Quaternion interpolated = lerp(a, b, t);
    double s = 1.0 / sqrt(30.0);
    CompareQuaternions(Quaternion(1, 2, 3, 4) * s, interpolated);
  }

  Quaternion a(4, 3, 2, 1);
  Quaternion b(1, 2, 3, 4);
  CompareQuaternions(a.GetNormalized(), lerp(a, b, 0));
  CompareQuaternions(b.GetNormalized(), lerp(a, b, 1));
  CompareQuaternions(Quaternion(1, 1, 1, 1).GetNormalized(), lerp(a, b, 0.5));
}

TEST(QuaternionTest, Slerp) {
  Vector3 axis(1, 1, 1);
  double start_radians = -0.5;
  double stop_radians = 0.5;
  auto start = Quaternion::FromAngleAxis(start_radians, axis);
  auto stop = Quaternion::FromAngleAxis(stop_radians, axis);

  for (int i = 0; i < 100; ++i) {
    float t = static_cast<float>(i) / 100;
    double radians = (1.0 - t) * start_radians + t * stop_radians;
    auto expected = Quaternion::FromAngleAxis(radians, axis);
    Quaternion interpolated = Slerp(start, stop, t);
    EXPECT_NEAR(expected.x, interpolated.x, TestEpsilon);
    EXPECT_NEAR(expected.y, interpolated.y, TestEpsilon);
    EXPECT_NEAR(expected.z, interpolated.z, TestEpsilon);
    EXPECT_NEAR(expected.w, interpolated.w, TestEpsilon);
  }
}

TEST(QuaternionTest, SlerpOppositeAngles) {
  Vector3 axis(1, 1, 1);
  double start_radians = -Angle::RightInRadians;
  double stop_radians = Angle::RightInRadians;
  auto start = Quaternion::FromAngleAxis(start_radians, axis);
  auto stop = Quaternion::FromAngleAxis(stop_radians, axis);

  // When quaternions are pointed in the fully opposite direction, this is
  // ambiguous, so we rotate as per https://www.w3.org/TR/css-transforms-1/
  auto expected = Quaternion::FromAngleAxis(0, axis);

  Quaternion interpolated = Slerp(start, stop, 0.5f);
  EXPECT_NEAR(expected.x, interpolated.x, TestEpsilon);
  EXPECT_NEAR(expected.y, interpolated.y, TestEpsilon);
  EXPECT_NEAR(expected.z, interpolated.z, TestEpsilon);
  EXPECT_NEAR(expected.w, interpolated.w, TestEpsilon);
}

} // namespace
} // namespace stp
