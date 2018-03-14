// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_RCPTR_H_
#define STP_BASE_MEMORY_RCPTR_H_

#include "Base/Memory/Rc.h"

namespace stp {

template<class T>
class RcPtr;

template<class T>
class RcPtr {
 public:
  RcPtr() : ptr_(nullptr) {}
  ~RcPtr() { decRefIfNotNull(ptr_); }

  RcPtr(RcPtr&& o) : ptr_(o.leakPtr()) {}
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr(RcPtr<U>&& o) : ptr_(o.leakPtr()) {}
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr(Rc<U>&& o) : ptr_(&o.leakRef()) {}

  RcPtr& operator=(RcPtr&& o) { return assignMove(o.leakPtr()); }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr& operator=(RcPtr<U>&& o) { return assignMove(o.leakPtr()); }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr& operator=(Rc<U>&& o) { return assignMove(&o.leakRef()); }

  RcPtr(const RcPtr& o) : ptr_(o.ptr_) { incRefIfNotNull(ptr_); }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr(const RcPtr<U>& o) : ptr_(o.get()) { incRefIfNotNull(ptr_); }

  RcPtr& operator=(const RcPtr& o) { reset(o.ptr_); return *this; }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr& operator=(const RcPtr<U>& o) { reset(o.ptr_); return *this; }

  RcPtr(T* ptr) : ptr_(ptr) { incRefIfNotNull(ptr); }
  RcPtr& operator=(T* ptr) { reset(ptr); return *this; }

  RcPtr(nullptr_t) : ptr_(nullptr) {}
  RcPtr& operator=(nullptr_t) { reset(); return *this; }

  [[nodiscard]] T* leakPtr() { return exchange(ptr_, nullptr); }

  void reset(T* new_ptr = nullptr) {
    incRefIfNotNull(new_ptr);
    decRefIfNotNull(exchange(ptr_, new_ptr));
  }

  T* get() const { return ptr_; }

  T& operator*() const { ASSERT(ptr_); return *ptr_; }
  T* operator->() const { ASSERT(ptr_); return ptr_; }

  explicit operator bool() const { return ptr_ != nullptr; }
  bool operator!() const { return !ptr_; }

  friend void swap(RcPtr& lhs, RcPtr& rhs) { swap(lhs.ptr_, rhs.ptr_); }

 private:
  friend RcPtr adoptRc<T>(T* ptr);

  enum AdoptTag { Adopt };

  T* ptr_;

  RcPtr(T* ptr, AdoptTag) : ptr_(ptr) {}

  ALWAYS_INLINE void incRefIfNotNull(T* ptr) {
    if (ptr)
      ptr->incRef();
  }

  ALWAYS_INLINE void decRefIfNotNull(T* ptr) {
    if (ptr)
      ptr->decRef();
  }

  RcPtr& assignMove(T* new_ptr) {
    decRefIfNotNull(exchange(ptr_, new_ptr));
    return *this;
  }

  friend bool operator==(const RcPtr& a, nullptr_t) { return !a; }
  friend bool operator!=(const RcPtr& a, nullptr_t) { return !!a; }

  friend bool operator==(nullptr_t, const RcPtr& b) { return !b; }
  friend bool operator!=(nullptr_t, const RcPtr& b) { return !!b; }
};

template<class T> struct TIsZeroConstructibleTmpl<RcPtr<T>> : TTrue {};
template<class T> struct TIsTriviallyRelocatableTmpl<RcPtr<T>> : TTrue {};
template<class T> struct TIsTriviallyEqualityComparableTmpl<RcPtr<T>> : TTrue {};

template<class T> inline RcPtr<T> adoptRc(T* ptr) {
  adoptedByRc(ptr);
  return RcPtr<T>(ptr, RcPtr<T>::Adopt);
}

template<class T> inline BorrowPtr<T> borrow(const RcPtr<T>& x) {
  return BorrowPtr<T>(x.get());
}
template<class T> BorrowPtr<T> borrow(RcPtr<T>&& x) = delete;

} // namespace stp

#endif // STP_BASE_MEMORY_RCPTR_H_
