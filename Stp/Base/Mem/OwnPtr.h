// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEM_OWNPTR_H_
#define STP_BASE_MEM_OWNPTR_H_

#include "Base/Debug/Assert.h"
#include "Base/Mem/Allocate.h"
#include "Base/Type/ComparableFwd.h"
#include "Base/Type/NullableFwd.h"
#include "Base/Type/Variable.h"

namespace stp {

template<class T, class TAllocator = DefaultAllocator>
class OwnPtr {
 public:
  static_assert(!TIsVoid<T>, "void type");

  typedef T ElementType;

  OwnPtr() = default;
  ~OwnPtr() { if (ptr_) Destroy(ptr_); }

  template<class U, TEnableIf<!TIsArray<U> && TIsConvertibleTo<U*, T*>>* = nullptr>
  OwnPtr(OwnPtr<U>&& u) noexcept : ptr_(u.Release()) {}
  template<class U, TEnableIf<!TIsArray<U> && TIsConvertibleTo<U*, T*>>* = nullptr>
  OwnPtr& operator=(OwnPtr<U>&& u) noexcept { Reset(u.Release()); return *this; }

  OwnPtr(nullptr_t) noexcept {}
  OwnPtr& operator=(nullptr_t) noexcept { Reset(); return *this; }

  explicit OwnPtr(T* ptr) noexcept : ptr_(ptr) { ASSERT(ptr_ != nullptr); }
  [[nodiscard]] T* Release() noexcept { return Exchange(ptr_, nullptr); }

  void Reset(T* new_ptr = nullptr) {
    T* tmp = Exchange(ptr_, new_ptr);
    if (tmp)
      Destroy(tmp);
  }

  T& operator*() const { ASSERT(ptr_); return *ptr_; }
  T* operator->() const { ASSERT(ptr_); return ptr_; }

  ALWAYS_INLINE T* get() const { return ptr_; }

  explicit operator bool() const { return ptr_ != nullptr; }

  template<typename... TArgs>
  static OwnPtr New(TArgs&&... args);

  friend void Swap(OwnPtr& l, OwnPtr& r) { Swap(l.ptr_, r.ptr_); }
  friend T* ToPointer(const OwnPtr& x) { return x.ptr_; }

  friend bool operator==(const OwnPtr& l, const OwnPtr& r) { return l.ptr_ == r.ptr_; }
  friend bool operator!=(const OwnPtr& l, const OwnPtr& r) { return l.ptr_ != r.ptr_; }

  friend bool operator==(const OwnPtr& l, T* r) { return l.ptr_ == r; }
  friend bool operator!=(const OwnPtr& l, T* r) { return l.ptr_ != r; }
  friend bool operator==(T* l, const OwnPtr& r) { return l == r.ptr_; }
  friend bool operator!=(T* l, const OwnPtr& r) { return l != r.ptr_; }

 private:
  void Destroy(T* ptr) {
    ptr->~T();
    TAllocator::Deallocate(ptr, isizeof(T));
  }

  T* ptr_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(OwnPtr);
};

template<class T, class TAllocator> bool operator==(const OwnPtr<T, TAllocator>&, nullptr_t) = delete;
template<class T, class TAllocator> bool operator!=(const OwnPtr<T, TAllocator>&, nullptr_t) = delete;
template<class T, class TAllocator> bool operator==(nullptr_t, const OwnPtr<T, TAllocator>&) = delete;
template<class T, class TAllocator> bool operator!=(nullptr_t, const OwnPtr<T, TAllocator>&) = delete;

template<typename T>
struct NullableTmpl<OwnPtr<T>> {
  typedef OwnPtr<T> Type;
};

template<typename T, typename TAllocator>
template<typename... TArgs>
inline OwnPtr<T, TAllocator> OwnPtr<T, TAllocator>::New(TArgs&&... args) {
  if (TIsNoexceptConstructible<T, TArgs...>) {
    void* raw_ptr = TAllocator::Allocate(isizeof(T));
    return OwnPtr(new(raw_ptr) T(Forward<TArgs>(args)...));
  } else {
    void* raw_ptr = TAllocator::Allocate(isizeof(T));
    T* ptr;
    try {
      ptr = new(raw_ptr) T(Forward<TArgs>(args)...);
    } catch (...) {
      TAllocator::Deallocate(ptr, isizeof(T));
      throw;
    }
    return OwnPtr(ptr);
  }
}

template<typename T, typename TAllocator>
struct TIsZeroConstructibleTmpl<OwnPtr<T, TAllocator>> : TTrue {};
template<typename T, typename TAllocator>
struct TIsTriviallyRelocatableTmpl<OwnPtr<T, TAllocator>> : TTrue {};
template<typename T, typename TAllocator>
struct TIsTriviallyEqualityComparableTmpl<OwnPtr<T, TAllocator>> : TTrue {};

template<typename T, class TAllocator>
class OwnPtr<T[], TAllocator> {
 public:
  static_assert(!TIsArray<T[]>, "C arrays disallowed, use List class instead");
};

// Helper to transfer ownership of a raw pointer to a OwnPtr<T>.
template<typename T>
inline OwnPtr<T> MakeOwnPtr(T* ptr) {
  return OwnPtr<T>(ptr);
}

} // namespace stp

#endif // STP_BASE_MEM_OWNPTR_H_
