// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_LINEARALLOCATOR_H_
#define STP_BASE_MEMORY_LINEARALLOCATOR_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Limits.h"
#include "Base/Type/Sign.h"

namespace stp {

class BASE_EXPORT LinearAllocator {
  DISALLOW_COPY_AND_ASSIGN(LinearAllocator);
 public:
  static constexpr int MinBlockSize = 1 << 10;
  static constexpr int MaxBlockSize = 1 << 30;

  explicit LinearAllocator(int min_block_size = MinBlockSize);
  ~LinearAllocator();

  void* tryAllocate(int size, int alignment);
  int freeRecent(void* ptr);
  bool contains(const void* ptr) const;

  void reset();
  void clear();

  int64_t getTotalCapacity() const { return total_capacity_; }
  int64_t getTotalUsed() const { return total_used_; }

  #if ASSERT_IS_ON
  int getBlockCount() const { return block_count_; }
  int64_t getTotalLost() const { return total_lost_; }
  #endif

 private:
  struct Block;

  Block* block_list_ = nullptr;
  int min_block_size_ = 0;
  int chunk_size_ = 0;
  int64_t total_capacity_ = 0;
  int64_t total_used_ = 0;
  #if ASSERT_IS_ON
  int block_count_ = 0;
  int64_t total_lost_ = 0;
  #endif

  Block* newBlock(int size);
  void freeChain(Block* block);
  void* tryAllocateWithinBlock(Block* block, int size, int alignment);

  #if ASSERT_IS_ON
  void validate() const;
  #else
  void validate() const {}
  #endif
};

} // namespace stp

#endif // STP_BASE_MEMORY_LINEARALLOCATOR_H_
