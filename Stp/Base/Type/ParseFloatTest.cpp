// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/ParseFloat.h"

#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {
namespace {

TEST(ParseFloatTest, TryParse) {
  static const struct {
    StringSpan input;
    double output;
    bool success;
  } cases[] = {
    {"0", 0.0, true},
    {"42", 42.0, true},
    {"-42", -42.0, true},
    {"123.45", 123.45, true},
    {"-123.45", -123.45, true},
    {"+123.45", 123.45, true},
    {"2.99792458e8", 299792458.0, true},
    {"149597870.691E+3", 149597870691.0, true},
    {"6.", 6.0, true},
    {"9e99999999999999999999", HUGE_VAL, true},
    {"-9e99999999999999999999", -HUGE_VAL, true},
    {"1e-2", 0.01, true},
    {"42 ", 42.0, false},
    {" 1e-2", 0.01, false},
    {"1e-2 ", 0.01, false},
    {"-1E-7", -0.0000001, true},
    {"01e02", 100, true},
    {"2.3e15", 2.3e15, true},
    {"\t\n\r -123.45e2", -12345.0, false},
    {"+123 e4", 123.0, false},
    {"123e ", 123.0, false},
    {"123e", 123.0, false},
    {" 2.99", 2.99, false},
    {"1e3.4", 1000.0, false},
    {"nothing", 0.0, false},
    {"-", 0.0, false},
    {"+", 0.0, false},
    {"", 0.0, false},
  };

  for (const auto& item : cases) {
    double output;
    EXPECT_EQ(item.success, TryParse(item.input, output));
    if (item.success) {
      EXPECT_DOUBLE_EQ(item.output, output);
    }
  }

  // One additional test to verify that conversion of numbers in strings with
  // embedded NUL characters.  The NUL and extra data after it should be
  // interpreted as junk after the number.
  double output;
  EXPECT_FALSE(TryParse("3.14\0" "159", output));
}

} // namespace
} // namespace stp
