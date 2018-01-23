// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Mem/AlignedMalloc.h"

#include "Base/Math/Alignment.h"

namespace stp {
namespace detail {

void* aligned_malloc(size_t size, size_t alignment) noexcept {
  ASSERT(size > 0);
  ASSERT(IsPowerOfTwo(alignment));
  ASSERT(alignment >= sizeof(void*));

  void* ptr;
  #if COMPILER(MSVC)
  ptr = _aligned_malloc(size, alignment);
  #else
  // posix_memalign() added in API level 16 for Android.
  if (posix_memalign(&ptr, alignment, size))
    ptr = nullptr;
  #endif

  ASSERT(IsAlignedTo(ptr, alignment));
  return ptr;
}

} // namespace detail
} // namespace stp
