// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/FlatSet.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(FlatSetTest, Add) {
  FlatSet<int> set;
  EXPECT_TRUE(set.IsEmpty());

  set.TryAdd(1);
  EXPECT_FALSE(set.IsEmpty());

  set.TryAdd(2);

  EXPECT_TRUE(set.Contains(1));
  EXPECT_TRUE(set.Contains(2));
  EXPECT_FALSE(set.TryAdd(1));
  EXPECT_FALSE(set.TryAdd(2));

  EXPECT_FALSE(set.Contains(5));
  EXPECT_TRUE(set.TryAdd(5));
  EXPECT_TRUE(set.Contains(5));

  EXPECT_TRUE(set.TryAdd(4));
  EXPECT_TRUE(set.Contains(4));

  EXPECT_FALSE(set.Contains(3));
}

TEST(FlatSetTest, Remove) {
  FlatSet<int> set;
  set.TryAdd(1);
  set.TryAdd(5);
  set.TryAdd(3);
  EXPECT_FALSE(set.TryRemove(4));
  set.TryAdd(4);
  EXPECT_TRUE(set.TryRemove(4));
  EXPECT_FALSE(set.TryRemove(4));
  set.TryAdd(4);
  set.TryRemove(4);
}

TEST(FlatSetTest, String) {
  FlatSet<String> map;
  map.TryAdd(String("abc"));
  EXPECT_TRUE(map.Contains(StringSpan("abc")));
  EXPECT_FALSE(map.Contains(StringSpan("def")));
  map.TryAdd(String("def"));
}

} // namespace stp
