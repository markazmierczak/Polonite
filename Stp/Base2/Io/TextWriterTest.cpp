// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/TextWriter.h"

#include "Base/Io/StringWriter.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(TextWriterTest, Char16) {
  String str;
  StringWriter out(str);

  out.Write("abc");
  EXPECT_EQ(StringSpan("abc"), str);

  out.Write(u"def");
  EXPECT_EQ(StringSpan("abcdef"), str);

  out.Write("gh");
  out.Write(u"ij");
  EXPECT_EQ(StringSpan("abcdefghij"), str);
}

} // namespace stp
