// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/Base64.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(Base64Test, Basic) {
  auto input = BufferSpan("hello world");
  const StringSpan expected = "aGVsbG8gd29ybGQ=";

  String encoded = Base64::Encode(input);
  EXPECT_EQ(expected, encoded);

  Buffer decoded;
  EXPECT_TRUE(Base64::TryDecode(encoded, decoded));
  EXPECT_EQ(input, decoded);
}

} // namespace stp
