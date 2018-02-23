// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_REFCOUNTED_H_
#define STP_BASE_MEMORY_REFCOUNTED_H_

#include "Base/Memory/RefPtr.h"

namespace stp {

class RefCountedBase {
 public:
  RefCountedBase() noexcept {
    #ifndef NDEBUG
    ref_count_ = -1;
    #else
    ref_count_ = 1;
    #endif
  }

  ~RefCountedBase() {
    ASSERT(ref_count_ == 0, "RefCountedBase object deleted without calling decRef()");
  }

  bool hasOneRef() const noexcept { return ref_count_ == 1; }

  void incRef() const noexcept {
    ASSERT(ref_count_ > 0);
    ref_count_++;
  }

  #ifndef NDEBUG
  void verifyAdoption() noexcept {
    ASSERT(ref_count_ == -1);
    ref_count_ = 1;
  }
  #endif

 protected:
  bool decRefBase() const noexcept {
    ASSERT(ref_count_ > 0);
    return --ref_count_ == 0;
  }

 private:
  mutable int ref_count_;

  DISALLOW_COPY_AND_ASSIGN(RefCountedBase);
};

inline void adoptedByRefPtr(RefCountedBase* refed) noexcept {
  #ifndef NDEBUG
  refed->verifyAdoption();
  #endif
}

template<typename T>
class RefCounted : public RefCountedBase {
 public:
  void decRef() const noexcept {
    if (decRefBase())
      delete static_cast<const T*>(this);
  }

 protected:
  RefCounted() = default;
};

} // namespace stp

#endif // STP_BASE_MEMORY_REFCOUNTED_H_
