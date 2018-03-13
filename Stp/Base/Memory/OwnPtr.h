// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_OWNPTR_H_
#define STP_BASE_MEMORY_OWNPTR_H_

#include "Base/Debug/Assert.h"
#include "Base/Memory/Allocate.h"
#include "Base/Type/Variable.h"

namespace stp {

template<class T, class TAllocator = DefaultAllocator>
class OwnPtr {
  DISALLOW_COPY_AND_ASSIGN(OwnPtr);
 public:
  static_assert(!TIsVoid<T>, "void type");
  static_assert(!TIsArray<T>, "C arrays disallowed, use List class instead");

  typedef T ElementType;

  OwnPtr() = default;
  ~OwnPtr() { if (ptr_) destroy(ptr_); }

  template<class U, TEnableIf<!TIsArray<U> && TIsConvertibleTo<U*, T*>>* = nullptr>
  OwnPtr(OwnPtr<U>&& u) : ptr_(u.leakPtr()) {}
  template<class U, TEnableIf<!TIsArray<U> && TIsConvertibleTo<U*, T*>>* = nullptr>
  OwnPtr& operator=(OwnPtr<U>&& u) { reset(u.leakPtr()); return *this; }

  OwnPtr(nullptr_t) {}
  OwnPtr& operator=(nullptr_t) { reset(); return *this; }

  explicit OwnPtr(T* ptr) : ptr_(ptr) { ASSERT(ptr_ != nullptr); }
  [[nodiscard]] T* leakPtr() { return exchange(ptr_, nullptr); }

  void reset(T* new_ptr = nullptr) {
    T* tmp = exchange(ptr_, new_ptr);
    if (tmp)
      destroy(tmp);
  }

  T& operator*() const { ASSERT(ptr_); return *ptr_; }
  T* operator->() const { ASSERT(ptr_); return ptr_; }

  T* get() const { return ptr_; }

  explicit operator bool() const { return ptr_ != nullptr; }
  bool operator!() const { return !ptr_; }

  template<class... TArgs>
  static OwnPtr create(TArgs&&... args);

  friend void swap(OwnPtr& l, OwnPtr& r) { swap(l.ptr_, r.ptr_); }

  friend bool operator==(const OwnPtr& l, nullptr_t) { return l.ptr_ == nullptr; }
  friend bool operator!=(const OwnPtr& l, nullptr_t) { return l.ptr_ != nullptr; }
  friend bool operator==(nullptr_t, const OwnPtr& r) { return nullptr == r.ptr_; }
  friend bool operator!=(nullptr_t, const OwnPtr& r) { return nullptr != r.ptr_; }

 private:
  T* ptr_ = nullptr;

  void destroy(T* ptr) {
    destroyObject(*ptr);
    TAllocator::deallocate(ptr, isizeof(T));
  }
};

template<class T, class TAllocator>
template<class... TArgs>
inline OwnPtr<T, TAllocator> OwnPtr<T, TAllocator>::create(TArgs&&... args) {
  if constexpr (TIsNoexceptConstructible<T, TArgs...>) {
    void* raw_ptr = TAllocator::allocate(isizeof(T));
    return OwnPtr(new(raw_ptr) T(forward<TArgs>(args)...));
  } else {
    void* raw_ptr = TAllocator::allocate(isizeof(T));
    T* ptr;
    try {
      ptr = new(raw_ptr) T(forward<TArgs>(args)...);
    } catch (...) {
      TAllocator::deallocate(ptr, isizeof(T));
      throw;
    }
    return OwnPtr(ptr);
  }
}

template<class T, class TAllocator>
struct TIsZeroConstructibleTmpl<OwnPtr<T, TAllocator>> : TTrue {};
template<class T, class TAllocator>
struct TIsTriviallyRelocatableTmpl<OwnPtr<T, TAllocator>> : TTrue {};

// Helper to transfer ownership of a raw pointer to a OwnPtr<T>.
template<class T>
inline OwnPtr<T> makeOwnPtr(T* ptr) {
  return OwnPtr<T>(ptr);
}

template<class T>
inline BorrowPtr<T> borrow(const OwnPtr<T>& x) {
  return BorrowPtr<T>(x.get());
}

} // namespace stp

#endif // STP_BASE_MEMORY_OWNPTR_H_
