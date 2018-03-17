// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_OWNPTR_H_
#define STP_BASE_MEMORY_OWNPTR_H_

#include "Base/Memory/Own.h"

namespace stp {

template<class T>
class OwnPtr {
  DISALLOW_COPY_AND_ASSIGN(OwnPtr);
 public:
  static_assert(!TIsVoid<T>, "void type");
  static_assert(!TIsArray<T>, "C arrays disallowed, use List class instead");

  OwnPtr() = default;
  ~OwnPtr() { if (ptr_) delete ptr_; }

  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  OwnPtr(OwnPtr<U>&& u) noexcept : ptr_(u.leakPtr()) {}
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  OwnPtr& operator=(OwnPtr<U>&& u) noexcept { reset(u.leakPtr()); return *this; }

  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  OwnPtr(Own<U>&& u) noexcept : ptr_(&u.leakRef()) {}
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  OwnPtr& operator=(Own<U>&& u) noexcept { reset(&u.leakRef()); return *this; }

  OwnPtr(nullptr_t) noexcept {}
  OwnPtr& operator=(nullptr_t) noexcept { reset(); return *this; }

  explicit OwnPtr(T* ptr) noexcept : ptr_(ptr) { ASSERT(ptr_ != nullptr); }
  [[nodiscard]] T* leakPtr() noexcept { return exchange(ptr_, nullptr); }

  void reset(T* new_ptr = nullptr) noexcept {
    T* tmp = exchange(ptr_, new_ptr);
    if (tmp)
      delete tmp;
  }

  T& operator*() const noexcept { ASSERT(ptr_); return *ptr_; }
  T* operator->() const noexcept { ASSERT(ptr_); return ptr_; }

  T* get() const noexcept { return ptr_; }

  explicit operator bool() const noexcept { return ptr_ != nullptr; }
  bool operator!() const noexcept { return !ptr_; }

  template<class... TArgs>
  static OwnPtr create(TArgs&&... args);

  friend void swap(OwnPtr& l, OwnPtr& r) noexcept { swap(l.ptr_, r.ptr_); }

  friend bool operator==(const OwnPtr& l, nullptr_t) noexcept { return l.ptr_ == nullptr; }
  friend bool operator!=(const OwnPtr& l, nullptr_t) noexcept { return l.ptr_ != nullptr; }
  friend bool operator==(nullptr_t, const OwnPtr& r) noexcept { return nullptr == r.ptr_; }
  friend bool operator!=(nullptr_t, const OwnPtr& r) noexcept { return nullptr != r.ptr_; }

 private:
  T* ptr_ = nullptr;
};

template<class T> struct TIsZeroConstructibleTmpl<OwnPtr<T>> : TTrue {};
template<class T> struct TIsTriviallyRelocatableTmpl<OwnPtr<T>> : TTrue {};

template<class T> template<class... TArgs>
inline OwnPtr<T> OwnPtr<T>::create(TArgs&&... args) {
  return OwnPtr(new T(forward<TArgs>(args)...));
}

// Helper to transfer ownership of a raw pointer to a OwnPtr<T>.
template<class T> inline OwnPtr<T> makeOwnPtr(T* ptr) noexcept {
  return OwnPtr<T>(ptr);
}

template<class T> inline BorrowPtr<T> borrow(const OwnPtr<T>& x) noexcept {
  return BorrowPtr<T>(x.get());
}
template<class T> BorrowPtr<T> borrow(OwnPtr<T>&& x) = delete;

} // namespace stp

#endif // STP_BASE_MEMORY_OWNPTR_H_
