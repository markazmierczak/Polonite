// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Memory/Allocate.h"

#include "Base/Error/BasicExceptions.h"

namespace stp {

void* allocateMemory(int size) {
  void* ptr = tryAllocateMemory(size);
  if (!ptr)
    throw OutOfMemoryException();
  return ptr;
}

void* reallocateMemory(void* ptr, int size) {
  ptr = tryReallocateMemory(ptr, size);
  if (!ptr)
    throw OutOfMemoryException();
  return ptr;
}

} // namespace stp
