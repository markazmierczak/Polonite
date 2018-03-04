// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Memory/Allocate.h"

#include "Base/Debug/Assert.h"

namespace stp {

void* reallocateMemory(void* ptr, int size) {
  ptr = tryReallocateMemory(ptr, size);
  PANIC_IF(!ptr, "out of memory");
  return ptr;
}

} // namespace stp

void* operator new(size_t size) {
  void* ptr = malloc(size);
  PANIC_IF(!ptr, "out of memory");
  return ptr;
}

void* operator new[](size_t size) {
  void* ptr = malloc(size);
  PANIC_IF(!ptr, "out of memory");
  return ptr;
}

void* operator new(size_t size, const std::nothrow_t&) {
  return malloc(size);
}

void* operator new[](size_t size, const std::nothrow_t&) {
  return malloc(size);
}

void operator delete(void* p) {
  free(p);
}

void operator delete[](void* p) {
  free(p);
}

void operator delete(void* p, const std::nothrow_t&) {
  free(p);
}

void operator delete[](void* p, const std::nothrow_t&) {
  free(p);
}
