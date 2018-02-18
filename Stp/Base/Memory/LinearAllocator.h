// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_LINEARALLOCATOR_H_
#define STP_BASE_MEMORY_LINEARALLOCATOR_H_

#include "Base/Debug/Assert.h"
#include "Base/Error/BasicExceptions.h"
#include "Base/Type/Limits.h"
#include "Base/Type/Sign.h"

namespace stp {

// An allocator that internally allocates multi-kbyte buffers for placing
// objects in. It avoids the overhead of malloc when many objects are allocated.
// It is most useful when creating many small objects with a similar lifetime,
// and doesn't add significant overhead for large allocations.
//
// * destructors will not be called
class BASE_EXPORT LinearAllocator {
 private:
  struct Block;

 public:
  static constexpr size_t MinBlockSize = 1024;
  // Max block size must be decreased by size of Block structure.
  // Minus one to fail if negative number is passed.
  static constexpr size_t MaxBlockSize = Limits<size_t>::Max / 2 - 1;

  explicit LinearAllocator(size_t min_block_size = MinBlockSize);

  ~LinearAllocator();

  // Call this to deallocate the most-recently allocated ptr by Allocate().
  // On success, the number of bytes freed is returned, or 0 if the block could
  // not be unallocated. This is a hint to the underlying allocator that
  // the previous allocation may be reused, but the implementation is free
  // to ignore this call (and return 0).
  size_t FreeRecent(void* ptr);

  // Frees all blocks.
  // All pointers allocated through allocate() are invalidated (cannot be dereferenced).
  void Reset();
  // Like Reset() but preserves largest block.
  void Clear();

  void* TryAllocate(size_t size, size_t alignment);

  // Returns true if the specified address is within one of the chunks, and
  // has at least 1-byte following the address (i.e. if |ptr| points to the
  // end of a chunk, then Contains() will return false).
  bool Contains(const void* ptr) const;

  ALWAYS_INLINE size_t GetTotalCapacity() const { return total_capacity_; }
  ALWAYS_INLINE size_t GetTotalUsed() const { return total_used_; }

  #if ASSERT_IS_ON
  ALWAYS_INLINE size_t GetBlockCount() const { return block_count_; }
  ALWAYS_INLINE size_t GetTotalLost() const { return total_lost_; }
  #endif

 private:
  Block* NewBlock(size_t size);
  void FreeChain(Block* block);
  void* TryAllocateWithinBlock(Block* block, size_t size, size_t alignment);

  #if ASSERT_IS_ON
  void Validate() const;
  #else
  void Validate() const {}
  #endif

  Block* block_list_;
  size_t min_block_size_;
  size_t chunk_size_;
  size_t total_capacity_;
  size_t total_used_;
  #if ASSERT_IS_ON
  size_t block_count_;
  size_t total_lost_;
  #endif

  DISALLOW_COPY_AND_ASSIGN(LinearAllocator);
};

template<typename TValue, typename TSize>
inline TValue* TryAllocate(LinearAllocator& allocator, TSize count) {
  ASSERT(count > 0);
  auto ucount = ToUnsigned(count);
  if (UNLIKELY(Limits<size_t>::Max / sizeof(TValue) < ucount))
    return nullptr;
  return static_cast<TValue*>(
      allocator.TryAllocate(ucount * sizeof(TValue), alignof(TValue)));
}

template<typename TValue, typename TSize>
inline TValue* Allocate(LinearAllocator& allocator, TSize count) {
  TValue* ptr = TryAllocate<TValue, TSize>(allocator, count);
  if (!ptr)
    throw OutOfMemoryException();
  return ptr;
}

} // namespace stp

#endif // STP_BASE_MEMORY_LINEARALLOCATOR_H_
