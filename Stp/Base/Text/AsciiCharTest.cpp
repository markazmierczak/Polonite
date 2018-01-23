// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/AsciiChar.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(CharTest, HexDigitToNibble) {
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
    EXPECT_EQ(item.nibble, TryParseHexDigit(item.character));
}

TEST(CharTest, ToLower) {
  EXPECT_EQ('c', ToLowerAscii('C'));
  EXPECT_EQ('c', ToLowerAscii('c'));
  EXPECT_EQ('2', ToLowerAscii('2'));

  EXPECT_EQ(u'c', ToLowerAscii(u'C'));
  EXPECT_EQ(u'c', ToLowerAscii(u'c'));
  EXPECT_EQ(u'2', ToLowerAscii(u'2'));
}

TEST(CharTest, ToUpper) {
  EXPECT_EQ('C', ToUpperAscii('C'));
  EXPECT_EQ('C', ToUpperAscii('c'));
  EXPECT_EQ('2', ToUpperAscii('2'));

  EXPECT_EQ(u'C', ToUpperAscii(u'C'));
  EXPECT_EQ(u'C', ToUpperAscii(u'c'));
  EXPECT_EQ(u'2', ToUpperAscii(u'2'));
}

} // namespace stp
