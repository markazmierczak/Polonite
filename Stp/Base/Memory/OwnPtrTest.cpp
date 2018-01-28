// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Memory/OwnPtr.h"

#include "Base/Test/GTest.h"
#include "Base/Containers/List.h"

namespace stp {

static_assert(TIsZeroConstructible<OwnPtr<int>>, "!");
static_assert(TIsTriviallyRelocatable<OwnPtr<int>>, "!");
static_assert(TIsTriviallyEqualityComparable<OwnPtr<int>>, "!");

TEST(OwnPtrTest, NewScalar) {
  auto s = OwnPtr<String>::New();
  EXPECT_EQ(StringSpan(""), *s);

  auto s2 = OwnPtr<String>::New("test");
  EXPECT_EQ(StringSpan("test"), *s2);
}

TEST(OwnPtrTest, NewScalarWithMoveOnlyType) {
  using MoveOnly = OwnPtr<String>;
  auto p = OwnPtr<MoveOnly>::New(OwnPtr<String>::New("test"));
  EXPECT_EQ(StringSpan("test"), **p);
}

} // namespace stp
