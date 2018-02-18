// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/ParseInteger.h"

#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {
namespace {

template<typename T>
struct ParseIntegerTestInputs {
  StringSpan input;
  T output;
  ParseIntegerErrorCode error_code;
};

template<typename T, int N>
void ParseIntegerTester(
    const ParseIntegerTestInputs<T> (&cases)[N],
    ParseIntegerErrorCode (*parser)(StringSpan, T&)) {
  T output;
  for (const auto& item : cases) {
    output = item.output ^ 1;  // Ensure tryParse wrote something.
    EXPECT_EQ(item.error_code, parser(item.input, output));
    if (item.error_code == ParseIntegerErrorCode::Ok) {
      EXPECT_EQ(item.output, output);
    }
  }

  // One additional test to verify that conversion of numbers in strings with
  // embedded NUL characters.  The NUL and extra data after it should be
  // interpreted as junk after the number.
  EXPECT_EQ(ParseIntegerErrorCode::FormatError, parser(StringSpan("6\06"), output));
}

TEST(ParseIntegerTest, tryParse) {
  ParseIntegerTestInputs<int> cases[] = {
    {"0", 0, ParseIntegerErrorCode::Ok},
    {"42", 42, ParseIntegerErrorCode::Ok},
    {"42\x99", 42, ParseIntegerErrorCode::FormatError},
    {"\x99" "42\x99", 0, ParseIntegerErrorCode::FormatError},
    {"-2147483648", Limits<int>::Min, ParseIntegerErrorCode::Ok},
    {"2147483647", Limits<int>::Max, ParseIntegerErrorCode::Ok},
    {"", 0, ParseIntegerErrorCode::FormatError},
    {" 42", 42, ParseIntegerErrorCode::FormatError},
    {"42 ", 42, ParseIntegerErrorCode::FormatError},
    {"\t\n\r 42", 42, ParseIntegerErrorCode::FormatError},
    {"blah42", 0, ParseIntegerErrorCode::FormatError},
    {"42blah", 42, ParseIntegerErrorCode::FormatError},
    {"blah42blah", 0, ParseIntegerErrorCode::FormatError},
    {"-273.15", -273, ParseIntegerErrorCode::FormatError},
    {"+98.6", 98, ParseIntegerErrorCode::FormatError},
    {"--123", 0, ParseIntegerErrorCode::FormatError},
    {"++123", 0, ParseIntegerErrorCode::FormatError},
    {"-+123", 0, ParseIntegerErrorCode::FormatError},
    {"+-123", 0, ParseIntegerErrorCode::FormatError},
    {"-", 0, ParseIntegerErrorCode::FormatError},
    {"-2147483649", 0, ParseIntegerErrorCode::OverflowError},
    {"-99999999999", 0, ParseIntegerErrorCode::OverflowError},
    {"2147483648", 0, ParseIntegerErrorCode::OverflowError},
    {"99999999999", 0, ParseIntegerErrorCode::OverflowError},
  };
  ParseIntegerTester<int>(cases, &tryParse);
}

TEST(ParseIntegerTest, tryParseUInt) {
  ParseIntegerTestInputs<unsigned> cases[] = {
    {"0", 0, ParseIntegerErrorCode::Ok},
    {"42", 42, ParseIntegerErrorCode::Ok},
    {"42\x99", 42, ParseIntegerErrorCode::FormatError},
    {"\x99" "42\x99", 0, ParseIntegerErrorCode::FormatError},
    {"-2147483648", 0, ParseIntegerErrorCode::FormatError},
    {"2147483647", Limits<int>::Max, ParseIntegerErrorCode::Ok},
    {"", 0, ParseIntegerErrorCode::FormatError},
    {" 42", 42, ParseIntegerErrorCode::FormatError},
    {"42 ", 42, ParseIntegerErrorCode::FormatError},
    {"\t\n\r 42", 42, ParseIntegerErrorCode::FormatError},
    {"blah42", 0, ParseIntegerErrorCode::FormatError},
    {"42blah", 42, ParseIntegerErrorCode::FormatError},
    {"blah42blah", 0, ParseIntegerErrorCode::FormatError},
    {"-273.15", 0, ParseIntegerErrorCode::FormatError},
    {"+98.6", 98, ParseIntegerErrorCode::FormatError},
    {"--123", 0, ParseIntegerErrorCode::FormatError},
    {"++123", 0, ParseIntegerErrorCode::FormatError},
    {"-+123", 0, ParseIntegerErrorCode::FormatError},
    {"+-123", 0, ParseIntegerErrorCode::FormatError},
    {"-", 0, ParseIntegerErrorCode::FormatError},
    {"-2147483649", 0, ParseIntegerErrorCode::FormatError},
    {"-99999999999", 0, ParseIntegerErrorCode::FormatError},
    {"4294967295", Limits<unsigned>::Max, ParseIntegerErrorCode::Ok},
    {"4294967296", 0, ParseIntegerErrorCode::OverflowError},
    {"99999999999", 0, ParseIntegerErrorCode::OverflowError},
  };
  ParseIntegerTester<unsigned>(cases, &tryParse);
}

TEST(ParseIntegerTest, tryParse64) {
  ParseIntegerTestInputs<int64_t> cases[] = {
    {"0", 0, ParseIntegerErrorCode::Ok},
    {"42", 42, ParseIntegerErrorCode::Ok},
    {"-2147483648", Limits<int>::Min, ParseIntegerErrorCode::Ok},
    {"2147483647", Limits<int>::Max, ParseIntegerErrorCode::Ok},
    {"-2147483649", INT64_C(-2147483649), ParseIntegerErrorCode::Ok},
    {"-99999999999", INT64_C(-99999999999), ParseIntegerErrorCode::Ok},
    {"2147483648", INT64_C(2147483648), ParseIntegerErrorCode::Ok},
    {"99999999999", INT64_C(99999999999), ParseIntegerErrorCode::Ok},
    {"9223372036854775807", Limits<int64_t>::Max, ParseIntegerErrorCode::Ok},
    {"-9223372036854775808", Limits<int64_t>::Min, ParseIntegerErrorCode::Ok},
    {"09", 9, ParseIntegerErrorCode::Ok},
    {"-09", -9, ParseIntegerErrorCode::Ok},
    {"", 0, ParseIntegerErrorCode::FormatError},
    {" 42", 42, ParseIntegerErrorCode::FormatError},
    {"42 ", 42, ParseIntegerErrorCode::FormatError},
    {"0x42", 0, ParseIntegerErrorCode::FormatError},
    {"\t\n\r 42", 42, ParseIntegerErrorCode::FormatError},
    {"blah42", 0, ParseIntegerErrorCode::FormatError},
    {"42blah", 42, ParseIntegerErrorCode::FormatError},
    {"blah42blah", 0, ParseIntegerErrorCode::FormatError},
    {"-273.15", -273, ParseIntegerErrorCode::FormatError},
    {"+98.6", 98, ParseIntegerErrorCode::FormatError},
    {"--123", 0, ParseIntegerErrorCode::FormatError},
    {"++123", 0, ParseIntegerErrorCode::FormatError},
    {"-+123", 0, ParseIntegerErrorCode::FormatError},
    {"+-123", 0, ParseIntegerErrorCode::FormatError},
    {"-", 0, ParseIntegerErrorCode::FormatError},
    {"-9223372036854775809", 0, ParseIntegerErrorCode::OverflowError},
    {"-99999999999999999999", 0, ParseIntegerErrorCode::OverflowError},
    {"9223372036854775808", 0, ParseIntegerErrorCode::OverflowError},
    {"99999999999999999999", 0, ParseIntegerErrorCode::OverflowError},
  };
  ParseIntegerTester<int64_t>(cases, &tryParse);
}

TEST(ParseIntegerTest, tryParseUInt64) {
  ParseIntegerTestInputs<uint64_t> cases[] = {
    {"0", 0, ParseIntegerErrorCode::Ok},
    {"42", 42, ParseIntegerErrorCode::Ok},
    {"-2147483648", 0, ParseIntegerErrorCode::FormatError},
    {"2147483647", INT_MAX, ParseIntegerErrorCode::Ok},
    {"-2147483649", 0, ParseIntegerErrorCode::FormatError},
    {"-99999999999", 0, ParseIntegerErrorCode::FormatError},
    {"2147483648", UINT64_C(2147483648), ParseIntegerErrorCode::Ok},
    {"99999999999", UINT64_C(99999999999), ParseIntegerErrorCode::Ok},
    {"9223372036854775807", Limits<int64_t>::Max, ParseIntegerErrorCode::Ok},
    {"-9223372036854775808", 0, ParseIntegerErrorCode::FormatError},
    {"09", 9, ParseIntegerErrorCode::Ok},
    {"-09", 0, ParseIntegerErrorCode::FormatError},
    {"", 0, ParseIntegerErrorCode::FormatError},
    {" 42", 42, ParseIntegerErrorCode::FormatError},
    {"42 ", 42, ParseIntegerErrorCode::FormatError},
    {"0x42", 0, ParseIntegerErrorCode::FormatError},
    {"\t\n\r 42", 42, ParseIntegerErrorCode::FormatError},
    {"blah42", 0, ParseIntegerErrorCode::FormatError},
    {"42blah", 42, ParseIntegerErrorCode::FormatError},
    {"blah42blah", 0, ParseIntegerErrorCode::FormatError},
    {"-273.15", 0, ParseIntegerErrorCode::FormatError},
    {"+98.6", 98, ParseIntegerErrorCode::FormatError},
    {"--123", 0, ParseIntegerErrorCode::FormatError},
    {"++123", 0, ParseIntegerErrorCode::FormatError},
    {"-+123", 0, ParseIntegerErrorCode::FormatError},
    {"+-123", 0, ParseIntegerErrorCode::FormatError},
    {"-", 0, ParseIntegerErrorCode::FormatError},
    {"-9223372036854775809", 0, ParseIntegerErrorCode::FormatError},
    {"-99999999999999999999", 0, ParseIntegerErrorCode::FormatError},
    {"9223372036854775808", UINT64_C(9223372036854775808), ParseIntegerErrorCode::Ok},
    {"99999999999999999999", 0, ParseIntegerErrorCode::OverflowError},
    {"18446744073709551615", Limits<uint64_t>::Max, ParseIntegerErrorCode::Ok},
    {"18446744073709551616", 0, ParseIntegerErrorCode::OverflowError},
  };
  ParseIntegerTester<uint64_t>(cases, &tryParse);
}

TEST(ParseIntegerTest, tryParseHex) {
  ParseIntegerTestInputs<int> cases[] = {
    {"0", 0, ParseIntegerErrorCode::Ok},
    {"42", 66, ParseIntegerErrorCode::Ok},
    {"-42", -66, ParseIntegerErrorCode::Ok},
    {"+42", 66, ParseIntegerErrorCode::Ok},
    {"7fffffff", INT_MAX, ParseIntegerErrorCode::Ok},
    {"-80000000", INT_MIN, ParseIntegerErrorCode::Ok},
    {"80000000", INT_MAX, ParseIntegerErrorCode::OverflowError},  // Overflow test.
    {"-80000001", INT_MIN, ParseIntegerErrorCode::OverflowError},  // Underflow test.
    {"0x42", 66, ParseIntegerErrorCode::Ok},
    {"-0x42", -66, ParseIntegerErrorCode::Ok},
    {"+0x42", 66, ParseIntegerErrorCode::Ok},
    {"0x7fffffff", INT_MAX, ParseIntegerErrorCode::Ok},
    {"-0x80000000", INT_MIN, ParseIntegerErrorCode::Ok},
    {"-80000000", INT_MIN, ParseIntegerErrorCode::Ok},
    {"0x0f", 15, ParseIntegerErrorCode::Ok},
    {"0f", 15, ParseIntegerErrorCode::Ok},
    {" 45", 0x45, ParseIntegerErrorCode::FormatError},
    {"\t\n\r 0x45", 0x45, ParseIntegerErrorCode::FormatError},
    {" 45", 0x45, ParseIntegerErrorCode::FormatError},
    {"45 ", 0x45, ParseIntegerErrorCode::FormatError},
    {"45:", 0x45, ParseIntegerErrorCode::FormatError},
    {"efgh", 0xEF, ParseIntegerErrorCode::FormatError},
    {"0xefgh", 0xEF, ParseIntegerErrorCode::FormatError},
    {"hgfe", 0, ParseIntegerErrorCode::FormatError},
    {"-", 0, ParseIntegerErrorCode::FormatError},
    {"", 0, ParseIntegerErrorCode::FormatError},
    {"0x", 0, ParseIntegerErrorCode::FormatError},
  };
  ParseIntegerTester<int>(cases, &tryParseHex);
}

TEST(ParseIntegerTest, tryParseHexUint) {
  ParseIntegerTestInputs<uint32_t> cases[] = {
    {"0", 0, ParseIntegerErrorCode::Ok},
    {"42", 0x42, ParseIntegerErrorCode::Ok},
    {"-42", 0, ParseIntegerErrorCode::FormatError},
    {"+42", 0x42, ParseIntegerErrorCode::Ok},
    {"7fffffff", INT_MAX, ParseIntegerErrorCode::Ok},
    {"-80000000", 0, ParseIntegerErrorCode::FormatError},
    {"ffffffff", 0xFFFFFFFF, ParseIntegerErrorCode::Ok},
    {"DeadBeef", 0xDEADBEEF, ParseIntegerErrorCode::Ok},
    {"0x42", 0x42, ParseIntegerErrorCode::Ok},
    {"-0x42", 0, ParseIntegerErrorCode::FormatError},
    {"+0x42", 0x42, ParseIntegerErrorCode::Ok},
    {"0x7fffffff", INT_MAX, ParseIntegerErrorCode::Ok},
    {"-0x80000000", 0, ParseIntegerErrorCode::FormatError},
    {"0xffffffff", Limits<uint32_t>::Max, ParseIntegerErrorCode::Ok},
    {"0XDeadBeef", 0xDEADBEEF, ParseIntegerErrorCode::Ok},
    {"0x7fffffffffffffff", Limits<uint32_t>::Max, ParseIntegerErrorCode::OverflowError},  // Overflow test.
    {"-0x8000000000000000", 0, ParseIntegerErrorCode::FormatError},
    {"0x8000000000000000", Limits<uint32_t>::Max, ParseIntegerErrorCode::OverflowError},  // Overflow test.
    {"-0x8000000000000001", 0, ParseIntegerErrorCode::FormatError},
    {"0xFFFFFFFFFFFFFFFF", Limits<uint32_t>::Max, ParseIntegerErrorCode::OverflowError},  // Overflow test.
    {"FFFFFFFFFFFFFFFF", Limits<uint32_t>::Max, ParseIntegerErrorCode::OverflowError},  // Overflow test.
    {"0x0000000000000000", 0, ParseIntegerErrorCode::Ok},
    {"0000000000000000", 0, ParseIntegerErrorCode::Ok},
    {"1FFFFFFFFFFFFFFFF", Limits<uint32_t>::Max, ParseIntegerErrorCode::OverflowError},  // Overflow test.
    {"0x0f", 0x0F, ParseIntegerErrorCode::Ok},
    {"0f", 0x0F, ParseIntegerErrorCode::Ok},
    {" 45", 0x45, ParseIntegerErrorCode::FormatError},
    {"\t\n\r 0x45", 0x45, ParseIntegerErrorCode::FormatError},
    {" 45", 0x45, ParseIntegerErrorCode::FormatError},
    {"45 ", 0x45, ParseIntegerErrorCode::FormatError},
    {"45:", 0x45, ParseIntegerErrorCode::FormatError},
    {"efgh", 0xEF, ParseIntegerErrorCode::FormatError},
    {"0xefgh", 0xEF, ParseIntegerErrorCode::FormatError},
    {"hgfe", 0, ParseIntegerErrorCode::FormatError},
    {"-", 0, ParseIntegerErrorCode::FormatError},
    {"", 0, ParseIntegerErrorCode::FormatError},
    {"0x", 0, ParseIntegerErrorCode::FormatError},
  };
  ParseIntegerTester<uint32_t>(cases, &tryParseHex);
}

TEST(ParseIntegerTest, tryParseHex64) {
  ParseIntegerTestInputs<int64_t> cases[] = {
    {"0", 0, ParseIntegerErrorCode::Ok},
    {"42", 66, ParseIntegerErrorCode::Ok},
    {"-42", -66, ParseIntegerErrorCode::Ok},
    {"+42", 66, ParseIntegerErrorCode::Ok},
    {"40acd88557b", INT64_C(4444444448123), ParseIntegerErrorCode::Ok},
    {"7fffffff", INT_MAX, ParseIntegerErrorCode::Ok},
    {"-80000000", INT_MIN, ParseIntegerErrorCode::Ok},
    {"ffffffff", 0xFFFFFFFF, ParseIntegerErrorCode::Ok},
    {"DeadBeef", 0xDEADBEEF, ParseIntegerErrorCode::Ok},
    {"0x42", 66, ParseIntegerErrorCode::Ok},
    {"-0x42", -66, ParseIntegerErrorCode::Ok},
    {"+0x42", 66, ParseIntegerErrorCode::Ok},
    {"0x40acd88557b", INT64_C(4444444448123), ParseIntegerErrorCode::Ok},
    {"0x7fffffff", INT_MAX, ParseIntegerErrorCode::Ok},
    {"-0x80000000", INT_MIN, ParseIntegerErrorCode::Ok},
    {"0xffffffff", 0xFFFFFFFF, ParseIntegerErrorCode::Ok},
    {"0XDeadBeef", 0xDEADBEEF, ParseIntegerErrorCode::Ok},
    {"0x7fffffffffffffff", Limits<int64_t>::Max, ParseIntegerErrorCode::Ok},
    {"-0x8000000000000000", Limits<int64_t>::Min, ParseIntegerErrorCode::Ok},
    {"0x8000000000000000", Limits<int64_t>::Max, ParseIntegerErrorCode::OverflowError},  // Overflow test.
    {"-0x8000000000000001", Limits<int64_t>::Min, ParseIntegerErrorCode::OverflowError},  // Underflow test.
    {"0x0f", 15, ParseIntegerErrorCode::Ok},
    {"0f", 15, ParseIntegerErrorCode::Ok},
    {" 45", 0x45, ParseIntegerErrorCode::FormatError},
    {"\t\n\r 0x45", 0x45, ParseIntegerErrorCode::FormatError},
    {" 45", 0x45, ParseIntegerErrorCode::FormatError},
    {"45 ", 0x45, ParseIntegerErrorCode::FormatError},
    {"45:", 0x45, ParseIntegerErrorCode::FormatError},
    {"efgh", 0xEF, ParseIntegerErrorCode::FormatError},
    {"0xefgh", 0xEF, ParseIntegerErrorCode::FormatError},
    {"hgfe", 0, ParseIntegerErrorCode::FormatError},
    {"-", 0, ParseIntegerErrorCode::FormatError},
    {"", 0, ParseIntegerErrorCode::FormatError},
    {"0x", 0, ParseIntegerErrorCode::FormatError},
  };
  ParseIntegerTester<int64_t>(cases, &tryParseHex);
}

TEST(ParseIntegerTest, tryParseHexUint64) {
  ParseIntegerTestInputs<uint64_t> cases[] = {
    {"0", 0, ParseIntegerErrorCode::Ok},
    {"42", 66, ParseIntegerErrorCode::Ok},
    {"-42", 0, ParseIntegerErrorCode::FormatError},
    {"+42", 66, ParseIntegerErrorCode::Ok},
    {"40acd88557b", INT64_C(4444444448123), ParseIntegerErrorCode::Ok},
    {"7fffffff", INT_MAX, ParseIntegerErrorCode::Ok},
    {"-80000000", 0, ParseIntegerErrorCode::FormatError},
    {"ffffffff", 0xFFFFFFFF, ParseIntegerErrorCode::Ok},
    {"DeadBeef", 0xDEADBEEF, ParseIntegerErrorCode::Ok},
    {"0x42", 66, ParseIntegerErrorCode::Ok},
    {"-0x42", 0, ParseIntegerErrorCode::FormatError},
    {"+0x42", 66, ParseIntegerErrorCode::Ok},
    {"0x40acd88557b", INT64_C(4444444448123), ParseIntegerErrorCode::Ok},
    {"0x7fffffff", INT_MAX, ParseIntegerErrorCode::Ok},
    {"-0x80000000", 0, ParseIntegerErrorCode::FormatError},
    {"0xffffffff", 0xFFFFFFFF, ParseIntegerErrorCode::Ok},
    {"0XDeadBeef", 0xDEADBEEF, ParseIntegerErrorCode::Ok},
    {"0x7fffffffffffffff", Limits<int64_t>::Max, ParseIntegerErrorCode::Ok},
    {"-0x8000000000000000", 0, ParseIntegerErrorCode::FormatError},
    {"0x8000000000000000", UINT64_C(0x8000000000000000), ParseIntegerErrorCode::Ok},
    {"-0x8000000000000001", 0, ParseIntegerErrorCode::FormatError},
    {"0xFFFFFFFFFFFFFFFF", Limits<uint64_t>::Max, ParseIntegerErrorCode::Ok},
    {"FFFFFFFFFFFFFFFF", Limits<uint64_t>::Max, ParseIntegerErrorCode::Ok},
    {"0x0000000000000000", 0, ParseIntegerErrorCode::Ok},
    {"0000000000000000", 0, ParseIntegerErrorCode::Ok},
    {"1FFFFFFFFFFFFFFFF", Limits<uint64_t>::Max, ParseIntegerErrorCode::OverflowError},  // Overflow test.
    {"0x0f", 15, ParseIntegerErrorCode::Ok},
    {"0f", 15, ParseIntegerErrorCode::Ok},
    {" 45", 0x45, ParseIntegerErrorCode::FormatError},
    {"\t\n\r 0x45", 0x45, ParseIntegerErrorCode::FormatError},
    {" 45", 0x45, ParseIntegerErrorCode::FormatError},
    {"45 ", 0x45, ParseIntegerErrorCode::FormatError},
    {"45:", 0x45, ParseIntegerErrorCode::FormatError},
    {"efgh", 0xEF, ParseIntegerErrorCode::FormatError},
    {"0xefgh", 0xEF, ParseIntegerErrorCode::FormatError},
    {"hgfe", 0, ParseIntegerErrorCode::FormatError},
    {"-", 0, ParseIntegerErrorCode::FormatError},
    {"", 0, ParseIntegerErrorCode::FormatError},
    {"0x", 0, ParseIntegerErrorCode::FormatError},
  };
  ParseIntegerTester<uint64_t>(cases, &tryParseHex);
}

} // namespace
} // namespace stp
