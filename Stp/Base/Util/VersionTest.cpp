// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/Version.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(VersionTest, Parse) {
  static const struct {
    StringSpan input;
    int firstpart;
    bool success;
  } cases[] = {
    {"", 0, false},
    {" ", 0, false},
    {"\t", 0, false},
    {"\n", 0, false},
    {"  ", 0, false},
    {".", 0, false},
    {" . ", 0, false},
    {"0", 0, true},
    {"0.", 0, false},
    {"0.0", 0, true},
    {"-1.0", 0, false},
    {"1.-1.0", 0, false},
    {"1,--1.0", 0, false},
    {"+1.0", 0, false},
    {"1.+1.0", 0, false},
    {"1+1.0", 0, false},
    {"++1.0", 0, false},
    {"1.0a", 0, false},
    {"1.2.3.4", 1, true},
    {"02.1", 2, true},
    {"0.01", 0, true},
    {"f.1", 0, false},
    {"15.007.20011", 15, true},
    {"15.5.28.130162", 15, true},
  };
  for (const auto& test : cases) {
    Version version;
    EXPECT_EQ(test.success, tryParse(test.input, version));
    if (test.success) {
      EXPECT_EQ(test.firstpart, version.getMajor());
    }
  }

  auto version = parseTo<Version>("15.5.28.130162");
  EXPECT_EQ(15, version.getMajor());
  EXPECT_EQ(5, version.getMinor());
  EXPECT_EQ(28, version.getPartAt(2));
  EXPECT_EQ(130162, version.getPartAt(3));
}

TEST(VersionTest, Compare) {
  static const struct {
    StringSpan lhs;
    StringSpan rhs;
    int expected;
  } cases[] = {
    {"1.0", "1.0", 0},
    {"1.0", "0.0", 1},
    {"1.0", "2.0", -1},
    {"1.0", "1.1", -1},
    {"1.1", "1.0", 1},
    {"1.0", "1.0.1", -1},
    {"1.1", "1.0.1", 1},
    {"1.1", "1.0.1", 1},
    {"1.0.0", "1.0", 0},
    {"1.0.3", "1.0.20", -1},
    {"11.0.10", "15.007.20011", -1},
    {"11.0.10", "15.5.28.130162", -1},
  };
  for (const auto& test : cases) {
    auto lhs = parseTo<Version>(test.lhs);
    auto rhs = parseTo<Version>(test.rhs);
    EXPECT_EQ(test.expected, compare(lhs, rhs));

    switch (test.expected) {
      case -1:
        EXPECT_LT(lhs, rhs);
        EXPECT_LE(lhs, rhs);
        EXPECT_NE(lhs, rhs);
        EXPECT_FALSE(lhs == rhs);
        EXPECT_FALSE(lhs >= rhs);
        EXPECT_FALSE(lhs > rhs);
        break;
      case 0:
        EXPECT_FALSE(lhs < rhs);
        EXPECT_LE(lhs, rhs);
        EXPECT_FALSE(lhs != rhs);
        EXPECT_EQ(lhs, rhs);
        EXPECT_GE(lhs, rhs);
        EXPECT_FALSE(lhs > rhs);
        break;
      case 1:
        EXPECT_FALSE(lhs < rhs);
        EXPECT_FALSE(lhs <= rhs);
        EXPECT_NE(lhs, rhs);
        EXPECT_FALSE(lhs == rhs);
        EXPECT_GE(lhs, rhs);
        EXPECT_GT(lhs, rhs);
        break;
    }
  }
}

} // namespace stp
