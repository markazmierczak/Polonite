// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_RCPTR_H_
#define STP_BASE_MEMORY_RCPTR_H_

#include "Base/Memory/Rc.h"

namespace stp {

template<class T>
class RcPtr {
 public:
  RcPtr() noexcept : ptr_(nullptr) {}
  ~RcPtr() { decRefIfNotNull(ptr_); }

  RcPtr(RcPtr&& o) noexcept : ptr_(o.leakPtr()) {}
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr(RcPtr<U>&& o) noexcept : ptr_(o.leakPtr()) {}
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr(Rc<U>&& o) noexcept : ptr_(&o.leakRef()) {}

  RcPtr& operator=(RcPtr&& o) noexcept { return assignMove(o.leakPtr()); }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr& operator=(RcPtr<U>&& o) noexcept { return assignMove(o.leakPtr()); }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr& operator=(Rc<U>&& o) noexcept { return assignMove(&o.leakRef()); }

  RcPtr(const RcPtr& o) noexcept : ptr_(o.ptr_) { incRefIfNotNull(ptr_); }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr(const RcPtr<U>& o) noexcept : ptr_(o.get()) { incRefIfNotNull(ptr_); }

  RcPtr& operator=(const RcPtr& o) noexcept { reset(o.ptr_); return *this; }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RcPtr& operator=(const RcPtr<U>& o) noexcept { reset(o.ptr_); return *this; }

  RcPtr(T* ptr) noexcept : ptr_(ptr) { incRefIfNotNull(ptr); }
  RcPtr& operator=(T* ptr) noexcept { reset(ptr); return *this; }

  RcPtr(nullptr_t) noexcept : ptr_(nullptr) {}
  RcPtr& operator=(nullptr_t) noexcept { reset(); return *this; }

  [[nodiscard]] T* leakPtr() noexcept { return exchange(ptr_, nullptr); }

  void reset(T* new_ptr = nullptr) noexcept {
    incRefIfNotNull(new_ptr);
    decRefIfNotNull(exchange(ptr_, new_ptr));
  }

  T* get() const noexcept { return ptr_; }

  T& operator*() const noexcept { ASSERT(ptr_); return *ptr_; }
  T* operator->() const noexcept { ASSERT(ptr_); return ptr_; }

  explicit operator bool() const noexcept { return ptr_ != nullptr; }
  bool operator!() const noexcept { return !ptr_; }

  friend void swap(RcPtr& lhs, RcPtr& rhs) noexcept { swap(lhs.ptr_, rhs.ptr_); }

 private:
  friend RcPtr adoptRc<T>(T* ptr) noexcept;

  enum AdoptTag { Adopt };

  T* ptr_;

  RcPtr(T* ptr, AdoptTag) noexcept : ptr_(ptr) {}

  ALWAYS_INLINE void incRefIfNotNull(T* ptr) noexcept {
    if (ptr)
      ptr->incRef();
  }

  ALWAYS_INLINE void decRefIfNotNull(T* ptr) noexcept {
    if (ptr)
      ptr->decRef();
  }

  RcPtr& assignMove(T* new_ptr) noexcept {
    decRefIfNotNull(exchange(ptr_, new_ptr));
    return *this;
  }

  friend bool operator==(const RcPtr& a, nullptr_t) noexcept { return !a; }
  friend bool operator!=(const RcPtr& a, nullptr_t) noexcept { return !!a; }

  friend bool operator==(nullptr_t, const RcPtr& b) noexcept { return !b; }
  friend bool operator!=(nullptr_t, const RcPtr& b) noexcept { return !!b; }
};

template<class T> struct TIsZeroConstructibleTmpl<RcPtr<T>> : TTrue {};
template<class T> struct TIsTriviallyRelocatableTmpl<RcPtr<T>> : TTrue {};
template<class T> struct TIsTriviallyEqualityComparableTmpl<RcPtr<T>> : TTrue {};

template<class T> inline RcPtr<T> adoptRc(T* ptr) noexcept {
  adoptedByRc(ptr);
  return RcPtr<T>(ptr, RcPtr<T>::Adopt);
}

template<class T> inline BorrowPtr<T> borrow(const RcPtr<T>& x) noexcept {
  return BorrowPtr<T>(x.get());
}
template<class T> BorrowPtr<T> borrow(RcPtr<T>&& x) = delete;

} // namespace stp

#endif // STP_BASE_MEMORY_RCPTR_H_
