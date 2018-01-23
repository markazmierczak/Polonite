// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEM_REFCOUNTED_H_
#define STP_BASE_MEM_REFCOUNTED_H_

#include "Base/Mem/RefPtr.h"

namespace stp {

class RefCountedBase {
 public:
  RefCountedBase() {
    #ifndef NDEBUG
    ref_count_ = -1;
    #else
    ref_count_ = 1;
    #endif
  }

  ~RefCountedBase() {
    ASSERT(ref_count_ == 0, "RefCountedBase object deleted without calling DecRef()");
  }

  bool HasOneRef() const { return ref_count_ == 1; }

  void IncRef() const {
    ASSERT(ref_count_ > 0);
    ref_count_++;
  }

  #ifndef NDEBUG
  void VerifyAdoption() {
    ASSERT(ref_count_ == -1);
    ref_count_ = 1;
  }
  #endif

 protected:
  bool DecRefBase() const {
    ASSERT(ref_count_ > 0);
    return --ref_count_ == 0;
  }

 private:
  mutable int ref_count_;

  DISALLOW_COPY_AND_ASSIGN(RefCountedBase);
};

inline void RefAdopted(RefCountedBase* refed) {
  #ifndef NDEBUG
  refed->VerifyAdoption();
  #endif
}

template<typename T>
class RefCounted : public RefCountedBase {
 public:
  void DecRef() const {
    if (DecRefBase())
      delete static_cast<const T*>(this);
  }

 protected:
  RefCounted() = default;
};

} // namespace stp

#endif // STP_BASE_MEM_REFCOUNTED_H_
