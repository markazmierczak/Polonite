// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Memory/AlignedMalloc.h"

#include "Base/Math/Alignment.h"

namespace stp {

void* tryAllocateAlignedMemory(int size, int alignment) noexcept {
  ASSERT(size > 0);
  ASSERT(isPowerOfTwo(alignment));
  ASSERT(alignment >= isizeof(void*));

  void* ptr;
  #if COMPILER(MSVC)
  ptr = _aligned_malloc(toUnsigned(size), toUnsigned(alignment));
  #else
  // posix_memalign() added in API level 16 for Android.
  if (posix_memalign(&ptr, toUnsigned(alignment), toUnsigned(size)))
    ptr = nullptr;
  #endif

  ASSERT(isAlignedTo(ptr, alignment));
  return ptr;
}

} // namespace stp
