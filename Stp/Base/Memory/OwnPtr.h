// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_OWNPTR_H_
#define STP_BASE_MEMORY_OWNPTR_H_

#include "Base/Debug/Assert.h"
#include "Base/Memory/Allocate.h"
#include "Base/Type/NullableFwd.h"
#include "Base/Type/Variable.h"

namespace stp {

template<class T, class TAllocator = DefaultAllocator>
class OwnPtr {
 public:
  static_assert(!TIsVoid<T>, "void type");
  static_assert(!TIsArray<T>, "C arrays disallowed, use List class instead");

  typedef T ElementType;

  OwnPtr() = default;
  ~OwnPtr() { if (ptr_) Destroy(ptr_); }

  template<class U, TEnableIf<!TIsArray<U> && TIsConvertibleTo<U*, T*>>* = nullptr>
  OwnPtr(OwnPtr<U>&& u) noexcept : ptr_(u.release()) {}
  template<class U, TEnableIf<!TIsArray<U> && TIsConvertibleTo<U*, T*>>* = nullptr>
  OwnPtr& operator=(OwnPtr<U>&& u) noexcept { Reset(u.release()); return *this; }

  OwnPtr(nullptr_t) noexcept {}
  OwnPtr& operator=(nullptr_t) noexcept { Reset(); return *this; }

  explicit OwnPtr(T* ptr) noexcept : ptr_(ptr) { ASSERT(ptr_ != nullptr); }
  [[nodiscard]] T* release() noexcept { return exchange(ptr_, nullptr); }

  void Reset(T* new_ptr = nullptr) {
    T* tmp = exchange(ptr_, new_ptr);
    if (tmp)
      Destroy(tmp);
  }

  T& operator*() const { ASSERT(ptr_); return *ptr_; }
  T* operator->() const { ASSERT(ptr_); return ptr_; }

  ALWAYS_INLINE T* get() const { return ptr_; }

  explicit operator bool() const { return ptr_ != nullptr; }

  template<typename... TArgs>
  static OwnPtr New(TArgs&&... args);

  friend void swap(OwnPtr& l, OwnPtr& r) { swap(l.ptr_, r.ptr_); }

  friend bool operator==(const OwnPtr& l, const OwnPtr& r) { return l.ptr_ == r.ptr_; }
  friend bool operator!=(const OwnPtr& l, const OwnPtr& r) { return l.ptr_ != r.ptr_; }

  friend bool operator==(const OwnPtr& l, T* r) { return l.ptr_ == r; }
  friend bool operator!=(const OwnPtr& l, T* r) { return l.ptr_ != r; }
  friend bool operator==(T* l, const OwnPtr& r) { return l == r.ptr_; }
  friend bool operator!=(T* l, const OwnPtr& r) { return l != r.ptr_; }

  friend bool operator==(const OwnPtr& l, nullptr_t) { return l.ptr_ == nullptr; }
  friend bool operator!=(const OwnPtr& l, nullptr_t) { return l.ptr_ != nullptr; }
  friend bool operator==(nullptr_t, const OwnPtr& r) { return nullptr == r.ptr_; }
  friend bool operator!=(nullptr_t, const OwnPtr& r) { return nullptr != r.ptr_; }

 private:
  void Destroy(T* ptr) {
    ptr->~T();
    TAllocator::deallocate(ptr, isizeof(T));
  }

  T* ptr_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(OwnPtr);
};

template<typename T>
struct NullableTmpl<OwnPtr<T>> {
  typedef OwnPtr<T> Type;
};

template<typename T, class TAllocator>
template<typename... TArgs>
inline OwnPtr<T, TAllocator> OwnPtr<T, TAllocator>::New(TArgs&&... args) {
  if constexpr (TIsNoexceptConstructible<T, TArgs...>) {
    void* raw_ptr = TAllocator::allocate(isizeof(T));
    return OwnPtr(new(raw_ptr) T(Forward<TArgs>(args)...));
  } else {
    void* raw_ptr = TAllocator::allocate(isizeof(T));
    T* ptr;
    try {
      ptr = new(raw_ptr) T(Forward<TArgs>(args)...);
    } catch (...) {
      TAllocator::deallocate(ptr, isizeof(T));
      throw;
    }
    return OwnPtr(ptr);
  }
}

template<typename T, class TAllocator>
struct TIsZeroConstructibleTmpl<OwnPtr<T, TAllocator>> : TTrue {};
template<typename T, class TAllocator>
struct TIsTriviallyRelocatableTmpl<OwnPtr<T, TAllocator>> : TTrue {};
template<typename T, class TAllocator>
struct TIsTriviallyEqualityComparableTmpl<OwnPtr<T, TAllocator>> : TTrue {};

// Helper to transfer ownership of a raw pointer to a OwnPtr<T>.
template<typename T>
inline OwnPtr<T> MakeOwnPtr(T* ptr) {
  return OwnPtr<T>(ptr);
}

} // namespace stp

#endif // STP_BASE_MEMORY_OWNPTR_H_
