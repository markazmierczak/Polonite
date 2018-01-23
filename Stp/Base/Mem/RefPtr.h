// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEM_REFPTR_H_
#define STP_BASE_MEM_REFPTR_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/ComparableFwd.h"
#include "Base/Type/Variable.h"

namespace stp {

template<typename T>
class RefPtr;

template<typename T>
RefPtr<T> AdoptRef(T* ptr);

ALWAYS_INLINE void RefAdopted(const void*) {}

template<typename T>
class RefPtr {
 public:
  // TODO noexcept
  RefPtr() : ptr_(nullptr) {}
  RefPtr(nullptr_t) : ptr_(nullptr) {}
  RefPtr(T* ptr) : ptr_(ptr) { IncRefIfNotNull(ptr); }

  RefPtr(const RefPtr& o) : ptr_(o.ptr_) { IncRefIfNotNull(ptr_); }
  template<typename U>
  RefPtr(const RefPtr<U>& o) : ptr_(o.get()) { IncRefIfNotNull(ptr_); }
  RefPtr(RefPtr&& o) : ptr_(o.ptr_) { o.ptr_ = nullptr; }

  ~RefPtr() { DecRefIfNotNull(ptr_); }

  RefPtr& operator=(RefPtr&& o) {
    T* old_ptr = Exchange(ptr_, Exchange(o.ptr_, nullptr));
    DecRefIfNotNull(old_ptr);
    return *this;
  }
  RefPtr& operator=(const RefPtr& o) {
    T* old_ptr = Exchange(ptr_, o.ptr_);
    IncRefIfNotNull(ptr_);
    DecRefIfNotNull(old_ptr);
    return *this;
  }
  RefPtr& operator=(nullptr_t) {
    Reset();
    return *this;
  }

  T* Release() WARN_UNUSED_RESULT { return Exchange(ptr_, nullptr); }

  void Reset() { DecRefIfNotNull(Exchange(ptr_, nullptr)); }

  ALWAYS_INLINE T& operator*() const { return *ptr_; }
  ALWAYS_INLINE T* operator->() const { return ptr_; }

  ALWAYS_INLINE bool operator!() const { return !ptr_; }
  ALWAYS_INLINE explicit operator bool() const { return ptr_ != nullptr; }

  ALWAYS_INLINE void SwapWith(RefPtr& other) { Swap(ptr_, other.ptr_); }

  ALWAYS_INLINE T* get() const { return ptr_; }

 private:
  friend RefPtr AdoptRef<T>(T* ptr);

  enum AdoptRefEnum { AdoptValue };

  T* ptr_;

  RefPtr(T* ptr, AdoptRefEnum) : ptr_(ptr) {}

  ALWAYS_INLINE void IncRefIfNotNull(T* ptr) {
    if (ptr)
      ptr->IncRef();
  }

  ALWAYS_INLINE void DecRefIfNotNull(T* ptr) {
    if (ptr)
      ptr->DecRef();
  }

  template<typename U>
  friend bool operator==(const RefPtr<T>& a, const RefPtr<U>& b) { return a.get() == b.get(); }
  template<typename U>
  friend bool operator==(const RefPtr<T>& a, U* b) { return a.get() == b; }
  template<typename U>
  friend bool operator==(T* a, const RefPtr<U>& b) { return a == b.get(); }
  template<typename U>
  friend bool operator!=(const RefPtr<T>& a, const RefPtr<U>& b) { return a.get() != b.get(); }
  template<typename U>
  friend bool operator!=(const RefPtr<T>& a, U* b) { return a.get() != b; }
  template<typename U>
  friend bool operator!=(T* a, const RefPtr<U>& b) { return a != b.get(); }

  friend bool operator==(const RefPtr<T>& a, nullptr_t) { return !a; }
  friend bool operator!=(const RefPtr<T>& a, nullptr_t) { return a.get(); }

  friend bool operator==(nullptr_t, const RefPtr<T>& b) { return !b; }
  friend bool operator!=(nullptr_t, const RefPtr<T>& b) { return b.get(); }
};

template<typename T>
struct TIsZeroConstructibleTmpl<RefPtr<T>> : TTrue {};
template<typename T>
struct TIsTriviallyRelocatableTmpl<RefPtr<T>> : TTrue {};
template<typename T>
struct TIsTriviallyEqualityComparableTmpl<RefPtr<T>> : TTrue {};

template<typename T>
RefPtr<T> AdoptRef(T* ptr) {
  RefAdopted(ptr);
  return RefPtr<T>(ptr, RefPtr<T>::AdoptValue);
}

} // namespace stp

#endif // STP_BASE_MEM_REFPTR_H_
