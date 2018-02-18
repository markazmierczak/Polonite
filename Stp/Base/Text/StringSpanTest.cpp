// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/StringAlgo.h"

#include "Base/Test/GTest.h"

namespace stp {
namespace {

TEST(StringSpanTest, Basic) {
  {
    StringSpan s;
    EXPECT_EQ(0, s.size());
    EXPECT_TRUE(s.IsEmpty());
  }
  {
    StringSpan s = "abc";
    EXPECT_EQ(3, s.size());
    EXPECT_FALSE(s.IsEmpty());
    EXPECT_EQ('a', s[0]);
    EXPECT_EQ('b', s[1]);
    EXPECT_EQ('c', s[2]);
  }
}

TEST(StringSpanTest, FirstLast) {
  StringSpan s = "abc";
  EXPECT_EQ('a', s.getFirst());
  EXPECT_EQ('c', s.getLast());
}

TEST(StringSpanTest, RemovePrefixSuffix) {
  {
    StringSpan s = "abcd";
    s.RemovePrefix(2);
    EXPECT_EQ(StringSpan("cd"), s);
  }
  {
    StringSpan s = "abcd";
    s.RemoveSuffix(1);
    EXPECT_EQ(StringSpan("abc"), s);
  }
}

TEST(StringSpanTest, Slice) {
  {
    StringSpan s = "abcde";
    s = s.getSlice(1, 2);
    EXPECT_EQ(StringSpan("bc"), s);
  }
  // Past end.
  {
    StringSpan s = "abcde";
    s = s.getSlice(2);
    EXPECT_EQ(StringSpan("cde"), s);
  }
}

/* FIXME
TEST(StringSpanTest, Compare_Operators) {
  #define CMP_Y(op, x, y) EXPECT_TRUE(StringSpan(x) op y)
  #define CMP_N(op, x, y) EXPECT_FALSE(StringSpan(x) op y)

  CMP_Y(==, "",   "");
  CMP_Y(==, "a",  "a");
  CMP_Y(==, "aa", "aa");
  CMP_N(==, "a",  "");
  CMP_N(==, "",   "a");
  CMP_N(==, "a",  "b");
  CMP_N(==, "a",  "aa");
  CMP_N(==, "aa", "a");

  CMP_N(!=, "",   "");
  CMP_N(!=, "a",  "a");
  CMP_N(!=, "aa", "aa");
  CMP_Y(!=, "a",  "");
  CMP_Y(!=, "",   "a");
  CMP_Y(!=, "a",  "b");
  CMP_Y(!=, "a",  "aa");
  CMP_Y(!=, "aa", "a");

  CMP_Y(<, "a",  "b");
  CMP_Y(<, "a",  "aa");
  CMP_Y(<, "aa", "b");
  CMP_Y(<, "aa", "bb");
  CMP_N(<, "a",  "a");
  CMP_N(<, "b",  "a");
  CMP_N(<, "aa", "a");
  CMP_N(<, "b",  "aa");
  CMP_N(<, "bb", "aa");

  CMP_Y(<=, "a",  "a");
  CMP_Y(<=, "a",  "b");
  CMP_Y(<=, "a",  "aa");
  CMP_Y(<=, "aa", "b");
  CMP_Y(<=, "aa", "bb");
  CMP_N(<=, "b",  "a");
  CMP_N(<=, "aa", "a");
  CMP_N(<=, "b",  "aa");
  CMP_N(<=, "bb", "aa");

  CMP_N(>=, "a",  "b");
  CMP_N(>=, "a",  "aa");
  CMP_N(>=, "aa", "b");
  CMP_N(>=, "aa", "bb");
  CMP_Y(>=, "a",  "a");
  CMP_Y(>=, "b",  "a");
  CMP_Y(>=, "aa", "a");
  CMP_Y(>=, "b",  "aa");
  CMP_Y(>=, "bb", "aa");

  CMP_N(>, "a",  "a");
  CMP_N(>, "a",  "b");
  CMP_N(>, "a",  "aa");
  CMP_N(>, "aa", "b");
  CMP_N(>, "aa", "bb");
  CMP_Y(>, "b",  "a");
  CMP_Y(>, "aa", "a");
  CMP_Y(>, "b",  "aa");
  CMP_Y(>, "bb", "aa");
}

TEST(StringSpanTest, Compare_Ordinal) {
  #define COMPARE(x, y) StringSpan(x).CompareTo(y)

  EXPECT_EQ( 0, COMPARE("abc", "abc"));
  EXPECT_EQ( 1, COMPARE("abc", "abb"));
  EXPECT_EQ(-1, COMPARE("abb", "abc"));

  EXPECT_EQ( 2, COMPARE("abcde", "abc"));
  EXPECT_EQ(-2, COMPARE("abc", "abcde"));

  #undef COMPARE
}
TEST(StringSpanTest, StartsWith_Ordinal) {
  #define STARTS_WITH(x, y) StringSpan(x).StartsWith(y)

  EXPECT_TRUE(STARTS_WITH("abc", "abc"));
  EXPECT_TRUE(STARTS_WITH("abcde", "abc"));
  EXPECT_FALSE(STARTS_WITH("abcde", "aBc"));
  EXPECT_FALSE(STARTS_WITH("abc", "abcde"));

  #undef STARTS_WITH
}

TEST(StringSpanTest, EndsWith_Ordinal) {
  #define ENDS_WITH(x, y) StringSpan(x).EndsWith(y)

  EXPECT_TRUE(ENDS_WITH("abc", "abc"));
  EXPECT_TRUE(ENDS_WITH("abcde", "cde"));
  EXPECT_FALSE(ENDS_WITH("abcde", "CdE"));
  EXPECT_FALSE(ENDS_WITH("abc", "abcde"));
  EXPECT_FALSE(ENDS_WITH("", "a"));

  #undef ENDS_WITH
}

TEST(StringSpanTest, IndexOfString_Ordinal) {
  #define INDEX_OF(c) a.IndexOf(c)
  #define LAST_INDEX_OF(c) a.LastIndexOf(c)

  StringSpan a = "abccdef";

  EXPECT_EQ(-1, INDEX_OF("a b"));
  EXPECT_EQ(0, INDEX_OF("abc"));
  EXPECT_EQ(5, INDEX_OF("ef"));
  EXPECT_EQ(6, INDEX_OF("f"));

  EXPECT_EQ(-1, LAST_INDEX_OF("n"));
  EXPECT_EQ(1, LAST_INDEX_OF("bc"));
  EXPECT_EQ(3, LAST_INDEX_OF("c"));
  EXPECT_EQ(5, LAST_INDEX_OF("ef"));

  a = "";
  EXPECT_EQ(-1, LAST_INDEX_OF("ef"));

  #undef INDEX_OF
  #undef LAST_INDEX_OF
}

TEST(StringSpanTest, Contains_Ordinal) {
  #define CONTAINS(x, y) StringSpan(x).Contains(y)

  EXPECT_TRUE(CONTAINS("abcde", "abc"));
  EXPECT_TRUE(CONTAINS("abcde", "cde"));
  EXPECT_TRUE(CONTAINS("abcde", "bcd"));
  EXPECT_FALSE(CONTAINS("abcde", "a c"));

  #undef CONTAINS
}

TEST(StringSpanTest, CountChar_Ordinal) {
  #define COUNT(x, y) StringSpan(x, sizeof(x) - 1).Count(y)

  EXPECT_EQ(6, COUNT("abbabbabbabbabba", 'a'));
  EXPECT_EQ(2, COUNT("a\0bcd\0sd", '\0'));

  #undef COUNT
}

TEST(StringSpanTest, CountString_Ordinal) {
  #define COUNT(x, y) StringSpan(x).Count(y)

  EXPECT_EQ(3, COUNT("abbabbabbabbabba", "abba"));
  EXPECT_EQ(1, COUNT("abba", "abba"));
  EXPECT_EQ(1, COUNT("abba", "ba"));
  EXPECT_EQ(2, COUNT("abba", "b"));

  #undef COUNT
}

TEST(StringSpanTest, IndexOfAnyOf) {
  #define INDEX_ANY_OF(x, y) StringSpan(x).IndexOfAny(y)
  #define LAST_INDEX_ANY_OF(x, y) StringSpan(x).LastIndexOfAny(y)

  EXPECT_EQ(-1, INDEX_ANY_OF("abcd", "efghijk"));
  EXPECT_EQ(1, INDEX_ANY_OF("abbccd", "efghijkb"));

  EXPECT_EQ(-1, LAST_INDEX_ANY_OF("abcd", "efghijk"));
  EXPECT_EQ(3, LAST_INDEX_ANY_OF("abccd", "efghijkca"));

  #undef INDEX_ANY_OF
  #undef LAST_INDEX_ANY_OF
}

TEST(StringSpanTest, IsAscii) {
  static char char_ascii[] =
      "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
  static char16_t char16_ascii[] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'A',
      'B', 'C', 'D', 'E', 'F', '0', '1', '2', '3', '4', '5', '6',
      '7', '8', '9', '0', 'A', 'B', 'C', 'D', 'E', 'F', 0 };
  static wchar_t wchar_ascii[] =
      L"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";

  // Test a variety of the fragment start positions and lengths in order to make
  // sure that bit masking in IsAsciiString works correctly.
  // Also, test that a non-ASCII character will be detected regardless of its
  // position inside the string.
  {
    const int string_length = isizeofArray(char_ascii) - 1;
    for (int offset = 0; offset < 8; ++offset) {
      for (int len = 0, max_len = string_length - offset; len < max_len; ++len) {
        EXPECT_TRUE(StringSpan(char_ascii + offset, len).IsAscii());
        for (int char_pos = offset; char_pos < len; ++char_pos) {
          char_ascii[char_pos] |= '\x80';
          EXPECT_FALSE(StringSpan(char_ascii + offset, len).IsAscii());
          char_ascii[char_pos] &= ~'\x80';
        }
      }
    }
  }

  {
    const int string_length = isizeofArray(char16_ascii) - 1;
    for (int offset = 0; offset < 4; ++offset) {
      for (int len = 0, max_len = string_length - offset; len < max_len; ++len) {
        EXPECT_TRUE(String16Span(char16_ascii + offset, len).IsAscii());
        for (int char_pos = offset; char_pos < len; ++char_pos) {
          char16_ascii[char_pos] |= 0x80;
          EXPECT_FALSE(String16Span(char16_ascii + offset, len).IsAscii());
          char16_ascii[char_pos] &= ~0x80;
          // Also test when the upper half is non-zero.
          char16_ascii[char_pos] |= 0x100;
          EXPECT_FALSE(String16Span(char16_ascii + offset, len).IsAscii());
          char16_ascii[char_pos] &= ~0x100;
        }
      }
    }
  }
}
*/

} // namespace
} // namespace stp
