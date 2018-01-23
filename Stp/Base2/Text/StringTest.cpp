// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/List.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(StringTest, Basic) {
  {
    WString s;
    EXPECT_EQ(0, s.size());
    EXPECT_TRUE(s.IsEmpty());
    // Always zero terminated.
    ASSERT_NE(nullptr, ToNullTerminated(s));
    EXPECT_EQ(wchar_t(0), *ToNullTerminated(s));
  }
  {
    const wchar_t cstr[] = L"abecadlo";
    auto s = WString(cstr);
    EXPECT_EQ(ArraySizeOf(cstr) - 1, s.size());
    EXPECT_EQ(cstr, s);
    EXPECT_EQ(wchar_t(0), *(ToNullTerminated(s) + s.size()));

    for (int i = 0; i < ArraySizeOf(cstr) - 1; ++i)
      EXPECT_EQ(cstr[i], s[i]);
  }
}

TEST(StringTest, Append) {
  // Short to Short.
  {
    auto s = String("ab");
    s += "cd";
    EXPECT_EQ("abcd", s);
    EXPECT_EQ(char(0), *(ToNullTerminated(s) + s.size()));
  }
  // Short to Long.
  {
    auto s = String("ab");
    s += "cdefghijklmn012345678";
    EXPECT_EQ("abcdefghijklmn012345678", s);
    EXPECT_EQ(char(0), *(ToNullTerminated(s) + s.size()));
  }

  // From internal buffer.
  {
    auto s = String("abcdefghijklmnop");
    int old_capacity = s.capacity();
    s += s;
    EXPECT_NE(old_capacity, s.capacity());
    EXPECT_EQ("abcdefghijklmnopabcdefghijklmnop", s);
  }
}

TEST(StringTest, Concat) {
  List<StringSpan> l;
  l.Add("abcdef");
  l.Add("012345");
  l.Add("ABCDEF");
  String c = String::ConcatArray(l);
  EXPECT_EQ("abcdef012345ABCDEF", c);
}

} // namespace stp
