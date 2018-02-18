// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Memory/LinearAllocator.h"

#include "Base/Math/Alignment.h"
#include "Base/Test/GTest.h"

namespace stp {
namespace {

void CheckAlloc(
    const LinearAllocator& allocator,
    unsigned capacity, unsigned used, unsigned num_blocks) {
  EXPECT_GE(allocator.GetTotalCapacity(), capacity);
  EXPECT_EQ(allocator.GetTotalUsed(), used);
  #if ASSERT_IS_ON
  EXPECT_EQ(allocator.GetBlockCount(), num_blocks);
  #endif
}

void* SimpleAlloc(LinearAllocator* allocator, unsigned size) {
  void* ptr = allocator->TryAllocate(size, 1);
  CheckAlloc(*allocator, size, size, 1);
  EXPECT_TRUE(allocator->Contains(ptr));
  return ptr;
}

TEST(ContiguousAllocatorTest, Basic) {
  constexpr size_t MinBlockSize = LinearAllocator::MinBlockSize;

  LinearAllocator allocator;

  // check empty
  CheckAlloc(allocator, 0, 0, 0);
  EXPECT_FALSE(allocator.Contains(nullptr));
  EXPECT_FALSE(allocator.Contains(this));

  // reset on empty allocator
  allocator.Reset();
  CheckAlloc(allocator, 0, 0, 0);

  // rewind on empty allocator
  allocator.Clear();
  CheckAlloc(allocator, 0, 0, 0);

  // test reset when something is allocated
  unsigned size = MinBlockSize >> 1;
  void* ptr = SimpleAlloc(&allocator, size);

  allocator.Reset();
  CheckAlloc(allocator, 0, 0, 0);
  EXPECT_FALSE(allocator.Contains(ptr));

  // test rewind when something is allocated
  ptr = SimpleAlloc(&allocator, size);

  allocator.Clear();
  CheckAlloc(allocator, size, 0, 1);
  EXPECT_FALSE(allocator.Contains(ptr));

  // use the available block
  ptr = SimpleAlloc(&allocator, size);
  allocator.Reset();

  // test out allocating a second block
  ptr = SimpleAlloc(&allocator, size);

  ptr = allocator.TryAllocate(MinBlockSize, 1);
  CheckAlloc(allocator, 2*MinBlockSize, size+MinBlockSize, 2);
  EXPECT_TRUE(allocator.Contains(ptr));

  // test out unalloc
  unsigned freed = allocator.FreeRecent(ptr);
  EXPECT_EQ(freed, MinBlockSize);
  CheckAlloc(allocator, 2*MinBlockSize, size, 2);
  EXPECT_FALSE(allocator.Contains(ptr));
}

TEST(ContiguousAllocatorTest, Alignment) {
  LinearAllocator allocator;
  IgnoreResult(Allocate<uint8_t>(allocator, 1));
  IgnoreResult(Allocate<int>(allocator, 1));
  IgnoreResult(Allocate<uint8_t>(allocator, 1));

  EXPECT_EQ(allocator.GetTotalUsed(), 9u);

  allocator.Reset();
  double* double_ptr = Allocate<double>(allocator, 1);
  EXPECT_TRUE(IsAlignedTo(double_ptr, alignof(double)));
  uint8_t* byte_after_double_ptr = Allocate<uint8_t>(allocator, 1);
  EXPECT_TRUE(IsAlignedTo(byte_after_double_ptr, alignof(double)));
  double_ptr = Allocate<double>(allocator, 1);
  EXPECT_TRUE(IsAlignedTo(double_ptr, alignof(double)));
}

} // namespace
} // namespace stp
