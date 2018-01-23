// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/Buffer.h"

#include "Base/Test/GTest.h"
#include "Base/Text/Format.h"

namespace stp {

TEST(BufferTest, Empty) {
  Buffer empty;
  EXPECT_EQ(nullptr, empty.data());
  EXPECT_EQ(0, empty.size());
  EXPECT_TRUE(empty.IsEmpty());
}

TEST(BufferTest, TryParse) {
  const struct {
    StringSpan input;
    BufferSpan expected;
    bool success;
  } cases[] = {
    {"", BufferSpan(), true},
    {"0", BufferSpan(), false},  // odd number of characters fails
    {"00", BufferSpan("\0"), true},
    {"42", BufferSpan("\x42"), true},
    {"-42", BufferSpan(), false},  // any non-hex value fails
    {"+42", BufferSpan(), false},
    {"7fffffff", BufferSpan("\x7F\xFF\xFF\xFF"), true},
    {"80000000", BufferSpan("\x80\0\0\0"), true},
    {"deadbeef", BufferSpan("\xde\xad\xbe\xef"), true},
    {"DeadBeef", BufferSpan("\xde\xad\xbe\xef"), true},
    {"0x42", BufferSpan(), false},  // leading 0x fails (x is not hex)
    {"0f", BufferSpan("\xf"), true},
    {"45  ", BufferSpan("\x45"), false},
    {"efgh", BufferSpan("\xef"), false},
    {"0123456789ABCDEF012345", BufferSpan("\x01\x23\x45\x67\x89\xAB\xCD\xEF\x01\x23\x45"), true},
  };

  for (const auto& item : cases) {
    Buffer output;
    EXPECT_EQ(item.success, TryParse(item.input, output));
    if (item.success)
      EXPECT_EQ(item.expected, output);
  }
}

TEST(BufferTest, Format) {
  constexpr byte_t bytes[] = {0x01, 0xFF, 0x02, 0xFE, 0x03, 0x80, 0x81};
  auto formatted = FormattableToString(BufferSpan(bytes));
  EXPECT_EQ("01FF02FE038081", formatted);
}

} // namespace stp
