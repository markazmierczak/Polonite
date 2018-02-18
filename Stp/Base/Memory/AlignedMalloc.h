// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_ALIGNEDMALLOC_H_
#define STP_BASE_MEMORY_ALIGNEDMALLOC_H_

#include "Base/Debug/Assert.h"
#include "Base/Error/BasicExceptions.h"
#include "Base/Type/Limits.h"
#include "Base/Type/Sign.h"

#if COMPILER(MSVC)
# include <malloc.h>
#else
# include <stdlib.h>
#endif

namespace stp {

namespace detail {

BASE_EXPORT void* aligned_malloc(size_t size, size_t alignment) noexcept;

// Accepts null pointer - no action performed.
inline void aligned_free(void* ptr) noexcept {
  #if COMPILER(MSVC)
  _aligned_free(ptr);
  #else
  free(ptr);
  #endif
}

} // namespace detail

template<typename TValue, typename TSize>
inline TValue* TryAlignedAllocate(TSize count) noexcept {
  ASSERT(count > 0);
  auto ucount = ToUnsigned(count);
  if (UNLIKELY(Limits<size_t>::Max / sizeof(TValue) < ucount))
    return nullptr;
  return (TValue*)detail::aligned_malloc(count * sizeof(TValue), alignof(TValue));
}

template<typename T>
inline void AlignedFree(T* ptr) noexcept {
  detail::aligned_free(ptr);
}

template<typename TValue, typename TSize>
inline TValue* AlignedAllocate(TSize count) {
  TValue* ptr = TryAlignedAllocate<TValue, TSize>(count);
  if (!ptr)
    throw OutOfMemoryException();
  return ptr;
}

template<typename T>
class AlignedAllocator {
 public:
  static void* allocate(int size) { return detail::aligned_malloc(ToUnsigned(size), alignof(T)); }
  static void deallocate(void* ptr, int size) { detail::aligned_free(ptr); }
};

} // namespace stp

#endif // STP_BASE_MEMORY_ALIGNEDMALLOC_H_
