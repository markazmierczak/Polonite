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
  auto s = OwnPtr<String>::create();
  EXPECT_EQ("", *s);

  auto s2 = OwnPtr<String>::create("test");
  EXPECT_EQ("test", *s2);
}

TEST(OwnPtrTest, NewScalarWithMoveOnlyType) {
  using MoveOnly = OwnPtr<String>;
  auto p = OwnPtr<MoveOnly>::create(OwnPtr<String>::create("test"));
  EXPECT_EQ("test", **p);
}

} // namespace stp
