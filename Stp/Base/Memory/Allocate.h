// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_ALLOCATE_H_
#define STP_BASE_MEMORY_ALLOCATE_H_

#include "Base/Type/Sign.h"

#include <new>
#include <stdlib.h>

namespace stp {

inline void* tryAllocateMemory(int size) noexcept {
  return ::malloc(toUnsigned(size));
}
inline void* tryReallocateMemory(void* ptr, int size) noexcept {
  return ::realloc(ptr, toUnsigned(size));
}
inline void freeMemory(void* ptr) noexcept { ::free(ptr); }

BASE_EXPORT void* allocateMemory(int size);
BASE_EXPORT void* reallocateMemory(void* ptr, int size);

class DefaultAllocator {
 public:
  static void* allocate(int size) {
    return allocateMemory(size);
  }
  static void deallocate(void* ptr, int size) noexcept {
    return freeMemory(ptr);
  }
};

} // namespace stp

#endif // STP_BASE_MEMORY_ALLOCATE_H_
