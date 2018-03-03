// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/FormatInteger.h"

#include "Base/String/String.h"
#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {
namespace {

template<typename TInteger>
struct FormatIntegerTest {
  TInteger num;
  StringSpan sexpected;
  StringSpan uexpected;
};

template<typename T>
static inline String formatInteger(T value) {
  FormatIntegerBuffer<T> buffer;
  return String(FormatInteger(value, buffer));
}

TEST(FormatIntegerTest, Basic) {
  static const FormatIntegerTest<int> IntTests[] = {
    { 0, "0", "0" },
    { -1, "-1", "4294967295" },
    { Limits<int>::Max,  "2147483647", "2147483647" },
    { Limits<int>::Min, "-2147483648", "2147483648" },
  };
  static const FormatIntegerTest<int64_t> Int64Tests[] = {
    {0, "0", "0"},
    {-1, "-1", "18446744073709551615"},
    {Limits<int64_t>::Max,  "9223372036854775807", "9223372036854775807"},
    {Limits<int64_t>::Min, "-9223372036854775808", "9223372036854775808"},
  };

  for (const auto& test : IntTests) {
    EXPECT_EQ(test.sexpected, formatInteger(test.num));
    EXPECT_EQ(test.uexpected, formatInteger(static_cast<unsigned>(test.num)));
  }
  for (const auto& test : Int64Tests) {
    EXPECT_EQ(test.sexpected, formatInteger(test.num));
    EXPECT_EQ(test.uexpected, formatInteger(static_cast<uint64_t>(test.num)));
  }
}

TEST(FormatIntegerTest, UnsignedAtEdge) {
  static const struct {
    uint64_t input;
    StringSpan expected;
  } cases[] = {
    {0, "0"},
    {42, "42"},
    {Limits<int>::Max, "2147483647"},
    {Limits<uint64_t>::Max, "18446744073709551615"},
  };

  for (const auto& test : cases) {
    EXPECT_EQ(test.expected, formatInteger(test.input));
  }
}

} // namespace
} // namespace stp
