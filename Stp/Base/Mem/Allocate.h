// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEM_ALLOCATE_H_
#define STP_BASE_MEM_ALLOCATE_H_

#include "Base/Debug/Assert.h"
#include "Base/Error/BasicExceptions.h"
#include "Base/Type/Limits.h"
#include "Base/Type/Sign.h"

#include <stdlib.h>

namespace stp {

// Will not work for over-aligned types (like SSE).
template<typename T, typename TSize = int>
inline T* TryAllocate(TSize count = 1) noexcept {
  ASSERT(count > 0);
  static_assert(alignof(T) <= alignof(max_align_t), "!");
  auto ucount = ToUnsigned(count);
  if (UNLIKELY(Limits<size_t>::Max / sizeof(T) < ucount))
    return nullptr;
  return (T*)::malloc(ucount * sizeof(T));
}

// |ptr| may be null.
template<typename T, typename TSize>
inline T* TryReallocate(T* ptr, TSize new_count) noexcept {
  ASSERT(new_count > 0);
  static_assert(alignof(T) <= alignof(max_align_t), "!");
  auto ucount = ToUnsigned(new_count);
  if (UNLIKELY(Limits<size_t>::Max / sizeof(T) < ucount))
    return nullptr;
  return (T*)::realloc(ptr, ucount * sizeof(T));
}

// |ptr| may be null.
inline void Free(void* ptr) noexcept { ::free(ptr); }

template<typename T, typename TSize = int>
inline T* Allocate(TSize count = 1) {
  T* ptr = TryAllocate<T, TSize>(count);
  if (!ptr)
    throw OutOfMemoryException();
  return ptr;
}

template<typename T, typename TSize>
inline T* Reallocate(T* ptr, TSize new_count) {
  ptr = TryReallocate<T, TSize>(ptr, new_count);
  if (!ptr)
    throw OutOfMemoryException();
  return ptr;
}

class DefaultAllocator {
 public:
  static void* Allocate(int size) {
    return operator new(ToUnsigned(size));
  }
  static void Deallocate(void* ptr, int size) {
    return operator delete(ptr);
  }
};

} // namespace stp

#endif // STP_BASE_MEM_ALLOCATE_H_
