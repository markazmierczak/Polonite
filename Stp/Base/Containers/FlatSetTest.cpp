// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/FlatSet.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(FlatSetTest, Add) {
  FlatSet<int> set;
  EXPECT_TRUE(set.isEmpty());

  set.tryAdd(1);
  EXPECT_FALSE(set.isEmpty());

  set.tryAdd(2);

  EXPECT_TRUE(set.contains(1));
  EXPECT_TRUE(set.contains(2));
  EXPECT_FALSE(set.tryAdd(1));
  EXPECT_FALSE(set.tryAdd(2));

  EXPECT_FALSE(set.contains(5));
  EXPECT_TRUE(set.tryAdd(5));
  EXPECT_TRUE(set.contains(5));

  EXPECT_TRUE(set.tryAdd(4));
  EXPECT_TRUE(set.contains(4));

  EXPECT_FALSE(set.contains(3));
}

TEST(FlatSetTest, Remove) {
  FlatSet<int> set;
  set.tryAdd(1);
  set.tryAdd(5);
  set.tryAdd(3);
  EXPECT_FALSE(set.TryRemove(4));
  set.tryAdd(4);
  EXPECT_TRUE(set.TryRemove(4));
  EXPECT_FALSE(set.TryRemove(4));
  set.tryAdd(4);
  set.TryRemove(4);
}

TEST(FlatSetTest, String) {
  FlatSet<String> map;
  map.tryAdd(String("abc"));
  EXPECT_TRUE(map.contains(StringSpan("abc")));
  EXPECT_FALSE(map.contains(StringSpan("def")));
  map.tryAdd(String("def"));
}

} // namespace stp
