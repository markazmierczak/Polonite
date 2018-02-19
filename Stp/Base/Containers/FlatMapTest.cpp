// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/FlatMap.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(FlatMapTest, Add) {
  FlatMap<int, int> map;
  EXPECT_TRUE(map.isEmpty());

  map.TryAdd(1, 2);
  EXPECT_FALSE(map.isEmpty());

  map.TryAdd(2, 3);

  EXPECT_EQ(2, map[1]);
  EXPECT_EQ(3, map[2]);
  EXPECT_FALSE(map.TryAdd(1, 0));
  EXPECT_FALSE(map.TryAdd(2, 0));
  EXPECT_TRUE(map.TryAdd(5, 0));
  EXPECT_EQ(0, map[5]);

  EXPECT_TRUE(map.TryAdd(4, 8));
  EXPECT_EQ(8, map[4]);

  EXPECT_FALSE(map.containsKey(3));
  EXPECT_TRUE(map.TryAdd(3, 7));
  EXPECT_EQ(7, map[3]);
  EXPECT_TRUE(map.containsKey(3));
}

TEST(FlatMapTest, Remove) {
  FlatMap<int, int> map;
  map.TryAdd(1, 1);
  map.TryAdd(5, 5);
  map.TryAdd(3, 3);
  EXPECT_FALSE(map.TryRemove(4));
  map.TryAdd(4, 4);
  EXPECT_TRUE(map.TryRemove(4));
  EXPECT_FALSE(map.TryRemove(4));
  map.TryAdd(4, 4);
  map.TryRemove(4);
}

TEST(FlatMapTest, String) {
  FlatMap<String, int> map;
  map.TryAdd("abc", 1);
  EXPECT_TRUE(map.containsKey("abc"));
  EXPECT_EQ(1, map["abc"]);
  map["abc"] = 2;
  EXPECT_EQ(2, map["abc"]);
}

} // namespace stp
