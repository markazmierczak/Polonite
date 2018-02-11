// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_POLYMORPHICALLOCATOR_H_
#define STP_BASE_MEMORY_POLYMORPHICALLOCATOR_H_

#include "Base/Type/Basic.h"

namespace stp {

class PolymorphicAllocator {
 public:
  virtual void* Allocate(int size) = 0;
  virtual void* Reallocate(void* ptr, int old_size, int new_size) = 0;
  virtual void Deallocate(void* ptr, int size) = 0;
};

} // namespace stp

#endif // STP_BASE_MEMORY_POLYMORPHICALLOCATOR_H_
