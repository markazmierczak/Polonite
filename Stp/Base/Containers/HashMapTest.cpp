// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/HashMap.h"

#include "Base/Containers/List.h"
#include "Base/Test/GTest.h"
#include "Base/Text/FormatMany.h"

namespace stp {

TEST(HashMapTest, Add) {
  HashMap<int, int> map;
  EXPECT_TRUE(map.IsEmpty());

  EXPECT_TRUE(map.TryAdd(1, 2));
  EXPECT_FALSE(map.IsEmpty());

  EXPECT_TRUE(map.TryAdd(2, 3));

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

TEST(HashMapTest, Remove) {
  HashMap<int, int> map;
  EXPECT_TRUE(map.TryAdd(1, 1));
  EXPECT_TRUE(map.TryAdd(5, 5));
  EXPECT_TRUE(map.TryAdd(3, 3));
  EXPECT_FALSE(map.TryRemove(4));
  EXPECT_TRUE(map.TryAdd(4, 4));
  EXPECT_TRUE(map.TryRemove(4));
  EXPECT_FALSE(map.TryRemove(4));
  EXPECT_TRUE(map.TryAdd(4, 4));
  EXPECT_TRUE(map.TryRemove(4));
}

TEST(HashMapTest, String) {
  HashMap<String, int> map;
  EXPECT_TRUE(map.TryAdd("abc", 1));
  EXPECT_TRUE(map.containsKey("abc"));
  EXPECT_EQ(1, map["abc"]);
  map["abc"] = 2;
  EXPECT_EQ(2, map["abc"]);
}

} // namespace stp
