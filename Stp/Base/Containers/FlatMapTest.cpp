// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/FlatMap.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(FlatMapTest, Add) {
  FlatMap<int, int> map;
  EXPECT_TRUE(map.isEmpty());

  map.tryAdd(1, 2);
  EXPECT_FALSE(map.isEmpty());

  map.tryAdd(2, 3);

  EXPECT_EQ(2, map[1]);
  EXPECT_EQ(3, map[2]);
  EXPECT_FALSE(map.tryAdd(1, 0));
  EXPECT_FALSE(map.tryAdd(2, 0));
  EXPECT_TRUE(map.tryAdd(5, 0));
  EXPECT_EQ(0, map[5]);

  EXPECT_TRUE(map.tryAdd(4, 8));
  EXPECT_EQ(8, map[4]);

  EXPECT_FALSE(map.containsKey(3));
  EXPECT_TRUE(map.tryAdd(3, 7));
  EXPECT_EQ(7, map[3]);
  EXPECT_TRUE(map.containsKey(3));
}

TEST(FlatMapTest, Remove) {
  FlatMap<int, int> map;
  map.tryAdd(1, 1);
  map.tryAdd(5, 5);
  map.tryAdd(3, 3);
  EXPECT_FALSE(map.tryRemove(4));
  map.tryAdd(4, 4);
  EXPECT_TRUE(map.tryRemove(4));
  EXPECT_FALSE(map.tryRemove(4));
  map.tryAdd(4, 4);
  map.tryRemove(4);
}

TEST(FlatMapTest, String) {
  FlatMap<String, int> map;
  map.tryAdd("abc", 1);
  EXPECT_TRUE(map.containsKey("abc"));
  EXPECT_EQ(1, map["abc"]);
  map["abc"] = 2;
  EXPECT_EQ(2, map["abc"]);
}

} // namespace stp
