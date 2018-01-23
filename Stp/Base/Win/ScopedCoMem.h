// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_WIN_SCOPEDCOMEM_H_
#define STP_BASE_WIN_SCOPEDCOMEM_H_

#include "Base/Debug/Assert.h"

#include <objbase.h>

namespace stp {
namespace win {

// Simple scoped memory releaser class for COM allocated memory.
// Example:
//   win::ScopedCoMem<ITEMIDLIST> file_item;
//   SHGetSomeInfo(&file_item, ...);
//   ...
//   return;  <-- memory released
template<typename T>
class ScopedCoMem {
 public:
  ScopedCoMem() : mem_ptr_(nullptr) {}
  ~ScopedCoMem() { Reset(nullptr); }

  T** operator&() {  
    ASSERT(mem_ptr_ == nullptr);  // To catch memory leaks.
    return &mem_ptr_;
  }

  T* operator->() {
    ASSERT(mem_ptr_ != nullptr);
    return mem_ptr_;
  }

  const T* operator->() const {
    ASSERT(mem_ptr_ != nullptr);
    return mem_ptr_;
  }

  void Reset(T* ptr) {
    if (mem_ptr_)
      CoTaskMemFree(mem_ptr_);
    mem_ptr_ = ptr;
  }

  T* get() const { return mem_ptr_; }

 private:
  T* mem_ptr_;

  DISALLOW_COPY_AND_ASSIGN(ScopedCoMem);
};

} // namespace win
} // namespace stp

#endif // STP_BASE_WIN_SCOPEDCOMEM_H_
