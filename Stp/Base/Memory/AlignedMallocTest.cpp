// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Memory/AlignedMalloc.h"

#include "Base/Memory/OwnPtr.h"
#include "Base/Test/GTest.h"

#define EXPECT_ALIGNED(ptr, align) \
    EXPECT_EQ(0u, reinterpret_cast<uintptr_t>(ptr) & (align - 1))

namespace stp {

TEST(AlignedMemory, DynamicAllocation) {
  void* p = detail::aligned_malloc(8, 8);
  EXPECT_TRUE(p);
  EXPECT_ALIGNED(p, 8);
  detail::aligned_free(p);

  p = detail::aligned_malloc(8, 16);
  EXPECT_TRUE(p);
  EXPECT_ALIGNED(p, 16);
  detail::aligned_free(p);

  p = detail::aligned_malloc(8, 256);
  EXPECT_TRUE(p);
  EXPECT_ALIGNED(p, 256);
  detail::aligned_free(p);

  p = detail::aligned_malloc(8, 4096);
  EXPECT_TRUE(p);
  EXPECT_ALIGNED(p, 4096);
  detail::aligned_free(p);
}

TEST(AlignedMemory, ScopedDynamicAllocation) {
  auto p = OwnPtr<double, AlignedAllocator<double>>::New(8);
  EXPECT_TRUE(p != nullptr);
  EXPECT_ALIGNED(p.get(), 4);
}

} // namespace stp
