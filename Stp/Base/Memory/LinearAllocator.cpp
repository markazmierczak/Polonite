// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Memory/LinearAllocator.h"

#include "Base/Math/Alignment.h"
#include "Base/Memory/Allocate.h"
#include "Base/Type/Sign.h"

#include <stdlib.h>

namespace stp {

struct LinearAllocator::Block {
  Block* next;
  char* free_ptr;
  size_t free_size;
  // data[] follows

  size_t GetSize() { return free_size + (free_ptr - GetData()); }

  void Reset() {
    next = nullptr;
    free_size = GetSize();
    free_ptr = GetData();
  }

  const char* GetData() const { return reinterpret_cast<const char*>(this + 1); }
  char* GetData() { return reinterpret_cast<char*>(this + 1); }

  bool contains(const void* addr) const {
    auto* ptr = reinterpret_cast<const char*>(addr);
    return GetData() <= ptr && ptr < free_ptr;
  }
};

LinearAllocator::LinearAllocator(size_t min_block_size) {
  ASSERT(min_block_size <= MaxBlockSize);

  if (min_block_size < MinBlockSize)
    min_block_size = MinBlockSize;

  block_list_ = nullptr;
  min_block_size_ = min_block_size;
  chunk_size_ = min_block_size_;
  total_capacity_ = 0;
  total_used_ = 0;
  #if ASSERT_IS_ON
  total_lost_ = 0;
  block_count_ = 0;
  #endif
}

LinearAllocator::~LinearAllocator() {
  FreeChain(block_list_);
}

void LinearAllocator::FreeChain(Block* block) {
  while (block) {
    Block* next = block->next;
    freeMemory(block);
    block = next;
  }
};

void LinearAllocator::Clear() {
  Validate();

  Block* largest = block_list_;

  if (largest) {
    Block* next;
    for (Block* cur = largest->next; cur; cur = next) {
      next = cur->next;
      if (cur->GetSize() > largest->GetSize()) {
        freeMemory(largest);
        largest = cur;
      } else {
        freeMemory(cur);
      }
    }

    largest->Reset();
    total_capacity_ = largest->GetSize();
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
  Validate();
}

void LinearAllocator::Reset() {
  FreeChain(block_list_);
  block_list_ = nullptr;
  chunk_size_ = min_block_size_; // Reset to our initial min_block_size_.
  total_capacity_ = 0;
  total_used_ = 0;
  #if ASSERT_IS_ON
  total_lost_ = 0;
  block_count_ = 0;
  #endif
}

void* LinearAllocator::TryAllocateWithinBlock(Block* block, size_t size, size_t alignment) {
  size_t padding;
  if (!AlignForward(block->free_ptr, alignment, size, block->free_size, &padding))
    return nullptr;

  void* ptr = block->free_ptr;
  block->free_ptr += size;
  block->free_size -= size;
  total_used_ += padding + size;

  Validate();
  return ptr;
}

void* LinearAllocator::TryAllocate(size_t size, size_t alignment) {
  Validate();

  void* rv = nullptr;
  if (block_list_)
    rv = TryAllocateWithinBlock(block_list_, size, alignment);

  if (rv == nullptr) {
    // Include alignment to guarantee further success.
    Block* block = NewBlock(size + alignment);
    #if ASSERT_IS_ON
    if (block_list_)
      total_lost_ += block_list_->free_size;
    #endif
    block->next = block_list_;
    block_list_ = block;

    rv = TryAllocateWithinBlock(block_list_, size, alignment);
  }
  ASSERT(rv != nullptr);
  return rv;
}

size_t LinearAllocator::FreeRecent(void* ptr) {
  Validate();

  size_t bytes = 0;
  Block* block = block_list_;
  if (block) {
    char* c_ptr = reinterpret_cast<char*>(ptr);
    ASSERT(block->contains(ptr));
    bytes = block->free_ptr - c_ptr;
    total_used_ -= bytes;
    block->free_size += bytes;
    block->free_ptr = c_ptr;
  }
  Validate();
  return bytes;
}

LinearAllocator::Block* LinearAllocator::NewBlock(size_t size) {
  ASSERT(size > 0);

  if (size < chunk_size_) {
    size = chunk_size_;
  } else if (size > MaxBlockSize) {
    throw LengthException();
  }

  auto* block = (Block*)allocateMemory(isizeof(Block) + size);

  block->free_size = size;
  block->free_ptr = block->GetData();

  total_capacity_ += size;
  #if ASSERT_IS_ON
  block_count_ += 1;
  #endif

  // Increase chunk size for next block.
  if (chunk_size_ < MaxBlockSize)
    chunk_size_ = AlignForward(chunk_size_ + (chunk_size_ >> 1), alignof(max_align_t));

  return block;
}

bool LinearAllocator::contains(const void* ptr) const {
  for (const Block* block = block_list_; block; block = block->next) {
    if (block->contains(ptr))
      return true;
  }
  return false;
}

#if ASSERT_IS_ON
void LinearAllocator::Validate() const {
  size_t computed_block_count = 0;
  size_t computed_total_capacity = 0;
  size_t computed_total_used = 0;
  size_t computed_total_lost = 0;
  size_t computed_total_available = 0;

  for (Block* block = block_list_; block; block = block->next) {
    ++computed_block_count;
    computed_total_capacity += block->GetSize();
    computed_total_used += block->free_ptr - block->GetData();
    if (block == block_list_)
      computed_total_available += block->free_size;
    else
      computed_total_lost += block->free_size;
  }

  ASSERT(block_count_ == computed_block_count);
  ASSERT(total_capacity_ == computed_total_capacity);
  ASSERT(total_used_ == computed_total_used);
  ASSERT(total_lost_ == computed_total_lost);

  size_t total_capacity = computed_total_used + computed_total_lost + computed_total_available;
  ASSERT(computed_total_capacity == total_capacity);
}
#endif // ASSERT_IS_ON

} // namespace stp
