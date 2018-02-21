// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_ALIGNEDMALLOC_H_
#define STP_BASE_MEMORY_ALIGNEDMALLOC_H_

#include "Base/Debug/Assert.h"

#if COMPILER(MSVC)
# include <malloc.h>
#else
# include <stdlib.h>
#endif

namespace stp {

BASE_EXPORT void* tryAllocateAlignedMemory(int size, int alignment) noexcept;

template<typename T>
inline void freeAlignedMemory(T* ptr) noexcept {
  #if COMPILER(MSVC)
  _alignedFreeImpl(ptr);
  #else
  free(ptr);
  #endif
}

template<int TAlignment>
class AlignedAllocator {
 public:
  static void* allocate(int size) { return tryAllocateAlignedMemory(size, TAlignment); }
  static void deallocate(void* ptr, int size) noexcept { freeAlignedMemory(ptr); }
};

} // namespace stp

#endif // STP_BASE_MEMORY_ALIGNEDMALLOC_H_
