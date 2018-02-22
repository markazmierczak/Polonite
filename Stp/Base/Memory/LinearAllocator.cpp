// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Memory/LinearAllocator.h"

#include "Base/Error/BasicExceptions.h"
#include "Base/Math/Alignment.h"
#include "Base/Memory/Allocate.h"

#include <stdlib.h>

namespace stp {

/**
 * @class LinearAllocator
 * An allocator that internally allocates multi-kbyte buffers for placing
 * objects in. It avoids the overhead of malloc when many objects are allocated.
 * It is most useful when creating many small objects with a similar lifetime,
 * and doesn't add significant overhead for large allocations.
 *
 * Note: no constructors are called by this class, and no destructors are
 *       called also! The client must taske care of this if needed.
 */

struct LinearAllocator::Block {
  Block* next;
  byte_t* free_ptr;
  int free_size;
  // data[] follows

  int getSize() { return free_size + (free_ptr - getData()); }

  void reset() {
    next = nullptr;
    free_size = getSize();
    free_ptr = getData();
  }

  const byte_t* getData() const { return reinterpret_cast<const byte_t*>(this + 1); }
  byte_t* getData() { return reinterpret_cast<byte_t*>(this + 1); }

  bool contains(const void* addr) const {
    auto* ptr = reinterpret_cast<const byte_t*>(addr);
    return getData() <= ptr && ptr < free_ptr;
  }
};

LinearAllocator::LinearAllocator(int min_block_size) {
  ASSERT(1 <= min_block_size && min_block_size <= MaxBlockSize);

  if (min_block_size < MinBlockSize)
    min_block_size = MinBlockSize;

  block_list_ = nullptr;
  min_block_size_ = min_block_size;
  chunk_size_ = min_block_size_;
}

LinearAllocator::~LinearAllocator() {
  freeChain(block_list_);
}

void LinearAllocator::freeChain(Block* block) {
  while (block) {
    Block* next = block->next;
    freeMemory(block);
    block = next;
  }
};

/**
 * Like @ref reset() but preserves largest block.
 */
void LinearAllocator::clear() {
  validate();

  Block* largest = block_list_;

  if (largest) {
    Block* next;
    for (Block* cur = largest->next; cur; cur = next) {
      next = cur->next;
      if (cur->getSize() > largest->getSize()) {
        freeMemory(largest);
        largest = cur;
      } else {
        freeMemory(cur);
      }
    }

    largest->reset();
    total_capacity_ = largest->getSize();
    #if ASSERT_IS_ON
    block_count_ = 1;
    #endif
  } else {
    total_capacity_ = 0;
    #if ASSERT_IS_ON
    block_count_ = 0;
    #endif
  }

  block_list_ = largest;
  chunk_size_ = min_block_size_; // Reset to our initial min_block_size_.
  total_used_ = 0;
  #if ASSERT_IS_ON
  total_lost_ = 0;
  #endif
  validate();
}

/**
 * Frees all blocks.
 * All pointers allocated through @ref allocate() are invalidated (cannot be dereferenced).
 */
void LinearAllocator::reset() {
  freeChain(block_list_);
  block_list_ = nullptr;
  chunk_size_ = min_block_size_; // Reset to our initial min_block_size_.
  total_capacity_ = 0;
  total_used_ = 0;
  #if ASSERT_IS_ON
  total_lost_ = 0;
  block_count_ = 0;
  #endif
}

void* LinearAllocator::tryAllocateWithinBlock(Block* block, int size, int alignment) {
  int padding;
  if (!alignForward(block->free_ptr, alignment, size, block->free_size, &padding))
    return nullptr;

  void* ptr = block->free_ptr;
  block->free_ptr += size;
  block->free_size -= size;
  total_used_ += padding + size;

  validate();
  return ptr;
}

void* LinearAllocator::tryAllocate(int size, int alignment) {
  validate();

  void* rv = nullptr;
  if (block_list_)
    rv = tryAllocateWithinBlock(block_list_, size, alignment);

  if (rv == nullptr) {
    // Include alignment to guarantee further success.
    Block* block = newBlock(size + alignment);
    #if ASSERT_IS_ON
    if (block_list_)
      total_lost_ += block_list_->free_size;
    #endif
    block->next = block_list_;
    block_list_ = block;

    rv = tryAllocateWithinBlock(block_list_, size, alignment);
  }
  ASSERT(rv != nullptr);
  return rv;
}

/**
 * Call this to deallocate the most-recently allocated @a ptr by @ref allocate().
 *
 * This is a hint to the underlying allocator that
 * the previous allocation may be reused, but the implementation is free
 * to ignore this call (and return 0).
 *
 * @return the number of bytes freed is returned on success. 0 otherwise.
 */
int LinearAllocator::freeRecent(void* ptr) {
  validate();

  int bytes = 0;
  Block* block = block_list_;
  if (block) {
    auto* c_ptr = reinterpret_cast<byte_t*>(ptr);
    ASSERT(block->contains(ptr));
    bytes = block->free_ptr - c_ptr;
    total_used_ -= bytes;
    block->free_size += bytes;
    block->free_ptr = c_ptr;
  }
  validate();
  return bytes;
}

LinearAllocator::Block* LinearAllocator::newBlock(int size) {
  ASSERT(size > 0);

  if (size < chunk_size_) {
    size = chunk_size_;
  } else if (size > MaxBlockSize) {
    throw LengthException();
  }

  auto* block = (Block*)allocateMemory(isizeof(Block) + size);

  block->free_size = size;
  block->free_ptr = block->getData();

  total_capacity_ += size;
  #if ASSERT_IS_ON
  block_count_ += 1;
  #endif

  // Increase chunk size for next block.
  if (chunk_size_ < MaxBlockSize) {
    int new_chunk_size = alignForward(chunk_size_ + (chunk_size_ >> 1), ialignof(max_align_t));
    chunk_size_ = min(new_chunk_size, MaxBlockSize);
  }
  return block;
}

/**
 * Returns true if the specified address is within one of the chunks, and
 * has at least 1-byte following the address (i.e. if @a ptr points to the
 * end of a chunk, then @ref contains() will return false).
 */
bool LinearAllocator::contains(const void* ptr) const {
  for (const Block* block = block_list_; block; block = block->next) {
    if (block->contains(ptr))
      return true;
  }
  return false;
}

#if ASSERT_IS_ON
void LinearAllocator::validate() const {
  int computed_block_count = 0;
  int computed_total_capacity = 0;
  int computed_total_used = 0;
  int computed_total_lost = 0;
  int computed_total_available = 0;

  for (Block* block = block_list_; block; block = block->next) {
    ++computed_block_count;
    computed_total_capacity += block->getSize();
    computed_total_used += block->free_ptr - block->getData();
    if (block == block_list_) {
      computed_total_available += block->free_size;
    } else {
      computed_total_lost += block->free_size;
    }
  }

  ASSERT(block_count_ == computed_block_count);
  ASSERT(total_capacity_ == computed_total_capacity);
  ASSERT(total_used_ == computed_total_used);
  ASSERT(total_lost_ == computed_total_lost);

  int total_capacity = computed_total_used + computed_total_lost + computed_total_available;
  ASSERT(computed_total_capacity == total_capacity);
}
#endif // ASSERT_IS_ON

} // namespace stp
