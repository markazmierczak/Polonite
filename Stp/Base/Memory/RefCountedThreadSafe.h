// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_REFCOUNTEDTHREADSAFE_H_
#define STP_BASE_MEMORY_REFCOUNTEDTHREADSAFE_H_

#include "Base/Memory/RcPtr.h"
#include "Base/Thread/AtomicRefCount.h"

namespace stp {

class BASE_EXPORT RefCountedThreadSafeBase {
 public:
  RefCountedThreadSafeBase() noexcept
      : ref_count_(1)
      #if ASSERT_IS_ON
      , in_dtor_(false)
      #endif
      {}

  ~RefCountedThreadSafeBase() noexcept {
    #if ASSERT_IS_ON
    ASSERT(in_dtor_, "RefCountedThreadSafeBase object deleted without calling decRef()");
    #endif
  }

  bool hasOneRef() const noexcept {
    return AtomicRefCountIsOne(&const_cast<RefCountedThreadSafeBase*>(this)->ref_count_);
  }

  void incRef() const noexcept {
    #if ASSERT_IS_ON
    ASSERT(!in_dtor_);
    #endif
    AtomicRefCountInc(&ref_count_);
  }

 protected:
  bool decRefBase() const noexcept {
    #if ASSERT_IS_ON
    ASSERT(!in_dtor_);
    ASSERT(!AtomicRefCountIsZero(&ref_count_));
    #endif
    if (!AtomicRefCountDec(&ref_count_)) {
      #if ASSERT_IS_ON
      in_dtor_ = true;
      #endif
      return true;
    }
    return false;
  }

 private:
  mutable AtomicRefCount ref_count_;
  #if ASSERT_IS_ON
  mutable bool in_dtor_;
  #endif

  DISALLOW_COPY_AND_ASSIGN(RefCountedThreadSafeBase);
};

template<typename T>
class RefCountedThreadSafe : public RefCountedThreadSafeBase {
 public:
  void decRef() const noexcept {
    if (decRefBase())
      delete static_cast<const T*>(this);
  }

 protected:
  RefCountedThreadSafe() = default;
};

} // namespace stp

#endif // STP_BASE_MEMORY_REFCOUNTEDTHREADSAFE_H_
