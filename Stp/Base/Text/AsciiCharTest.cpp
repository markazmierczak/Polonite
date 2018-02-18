// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/AsciiChar.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(AsciiCharTest, tryParseHexDigit) {
  struct {
    int nibble;
    char character;
  } cases[] = {
    {-1, ' '},
    {0, '0'},
    {1, '1'},
    {2, '2'},
    {3, '3'},
    {4, '4'},
    {5, '5'},
    {6, '6'},
    {7, '7'},
    {8, '8'},
    {9, '9'},
    {10, 'A'},
    {11, 'B'},
    {12, 'C'},
    {13, 'D'},
    {14, 'E'},
    {15, 'F'},
    // Verify the lower case as well.
    {10, 'a'},
    {11, 'b'},
    {12, 'c'},
    {13, 'd'},
    {14, 'e'},
    {15, 'f'},
    {-1, 'g'},
  };

  for (const auto& item : cases)
    EXPECT_EQ(item.nibble, tryParseHexDigit(item.character));
}

TEST(AsciiCharTest, toLower) {
  EXPECT_EQ('c', toLowerAscii('C'));
  EXPECT_EQ('c', toLowerAscii('c'));
  EXPECT_EQ('2', toLowerAscii('2'));

  EXPECT_EQ(u'c', toLowerAscii(u'C'));
  EXPECT_EQ(u'c', toLowerAscii(u'c'));
  EXPECT_EQ(u'2', toLowerAscii(u'2'));
}

TEST(AsciiCharTest, toUpper) {
  EXPECT_EQ('C', toUpperAscii('C'));
  EXPECT_EQ('C', toUpperAscii('c'));
  EXPECT_EQ('2', toUpperAscii('2'));

  EXPECT_EQ(u'C', toUpperAscii(u'C'));
  EXPECT_EQ(u'C', toUpperAscii(u'c'));
  EXPECT_EQ(u'2', toUpperAscii(u'2'));
}

} // namespace stp
