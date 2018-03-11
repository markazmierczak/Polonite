// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_REFCOUNTED_H_
#define STP_BASE_MEMORY_REFCOUNTED_H_

#include "Base/Memory/Rc.h"

namespace stp {

class RefCountedBase {
  DISALLOW_COPY_AND_ASSIGN(RefCountedBase);
 public:
  RefCountedBase() {
    #if ASSERT_IS_ON
    ref_count_ = -1;
    #else
    ref_count_ = 1;
    #endif
  }

  ~RefCountedBase() {
    ASSERT(ref_count_ == 0, "RefCountedBase object deleted without calling decRef()");
  }

  bool hasOneRef() const { return ref_count_ == 1; }

  void incRef() const {
    ASSERT(ref_count_ > 0);
    ref_count_++;
  }

  #if ASSERT_IS_ON
  void verifyAdoption() {
    ASSERT(ref_count_ == -1);
    ref_count_ = 1;
  }
  #endif

 protected:
  bool decRefBase() const {
    ASSERT(ref_count_ > 0);
    return --ref_count_ == 0;
  }

 private:
  mutable int ref_count_;
};

inline void adoptedByRc(RefCountedBase* refed) {
  #if ASSERT_IS_ON
  refed->verifyAdoption();
  #endif
}

template<typename T>
class RefCounted : public RefCountedBase {
 public:
  void decRef() const {
    if (decRefBase())
      delete static_cast<const T*>(this);
  }

 protected:
  RefCounted() = default;
};

} // namespace stp

#endif // STP_BASE_MEMORY_REFCOUNTED_H_
