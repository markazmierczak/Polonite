// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Memory/LinearAllocator.h"

#include "Base/Math/Alignment.h"
#include "Base/Test/GTest.h"

namespace stp {

static void checkAlloc(
    const LinearAllocator& allocator,
    int capacity, int used, int num_blocks) {
  EXPECT_GE(allocator.getTotalCapacity(), capacity);
  EXPECT_EQ(allocator.getTotalUsed(), used);
  #if ASSERT_IS_ON
  EXPECT_EQ(allocator.getBlockCount(), num_blocks);
  #endif
}

static void* simpleAlloc(LinearAllocator* allocator, int size) {
  void* ptr = allocator->tryAllocate(size, 1);
  checkAlloc(*allocator, size, size, 1);
  EXPECT_TRUE(allocator->contains(ptr));
  return ptr;
}

TEST(ContiguousAllocatorTest, basic) {
  constexpr int MinBlockSize = LinearAllocator::MinBlockSize;

  LinearAllocator allocator;

  // check empty
  checkAlloc(allocator, 0, 0, 0);
  EXPECT_FALSE(allocator.contains(nullptr));
  EXPECT_FALSE(allocator.contains(this));

  // reset on empty allocator
  allocator.reset();
  checkAlloc(allocator, 0, 0, 0);

  // rewind on empty allocator
  allocator.clear();
  checkAlloc(allocator, 0, 0, 0);

  // test reset when something is allocated
  int size = MinBlockSize >> 1;
  void* ptr = simpleAlloc(&allocator, size);

  allocator.reset();
  checkAlloc(allocator, 0, 0, 0);
  EXPECT_FALSE(allocator.contains(ptr));

  // test rewind when something is allocated
  ptr = simpleAlloc(&allocator, size);

  allocator.clear();
  checkAlloc(allocator, size, 0, 1);
  EXPECT_FALSE(allocator.contains(ptr));

  // use the available block
  ptr = simpleAlloc(&allocator, size);
  allocator.reset();

  // test out allocating a second block
  ptr = simpleAlloc(&allocator, size);

  ptr = allocator.tryAllocate(MinBlockSize, 1);
  checkAlloc(allocator, 2*MinBlockSize, size+MinBlockSize, 2);
  EXPECT_TRUE(allocator.contains(ptr));

  // test out unalloc
  int freed = allocator.freeRecent(ptr);
  EXPECT_EQ(freed, MinBlockSize);
  checkAlloc(allocator, 2*MinBlockSize, size, 2);
  EXPECT_FALSE(allocator.contains(ptr));
}

TEST(ContiguousAllocatorTest, alignment) {
  LinearAllocator allocator;
  ignoreResult(allocator.tryAllocate(1, 1));
  ignoreResult(allocator.tryAllocate(4, 4));
  ignoreResult(allocator.tryAllocate(1, 1));

  EXPECT_EQ(allocator.getTotalUsed(), 9u);
}

} // namespace stp
