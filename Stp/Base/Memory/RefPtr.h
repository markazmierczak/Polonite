// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_REFPTR_H_
#define STP_BASE_MEMORY_REFPTR_H_

#include "Base/Memory/Rc.h"

namespace stp {

template<typename T>
class RefPtr;

template<typename T>
class RefPtr {
 public:
  RefPtr() noexcept : ptr_(nullptr) {}
  ~RefPtr() { decRefIfNotNull(ptr_); }

  RefPtr(RefPtr&& o) noexcept : ptr_(o.leakPtr()) {}
  template<typename U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RefPtr(RefPtr<U>&& o) noexcept : ptr_(o.leakPtr()) {}
  template<typename U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RefPtr(Rc<U>&& o) noexcept : ptr_(&o.leakRef()) {}

  RefPtr& operator=(RefPtr&& o) noexcept { return assignMove(o.leakPtr()); }
  template<typename U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RefPtr& operator=(RefPtr<U>&& o) noexcept { return assignMove(o.leakPtr()); }
  template<typename U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RefPtr& operator=(Rc<U>&& o) noexcept { return assignMove(&o.leakRef()); }

  RefPtr(const RefPtr& o) noexcept : ptr_(o.ptr_) { incRefIfNotNull(ptr_); }
  template<typename U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RefPtr(const RefPtr<U>& o) noexcept : ptr_(o.get()) { incRefIfNotNull(ptr_); }

  RefPtr& operator=(const RefPtr& o) noexcept { reset(o.ptr_); return *this; }
  template<typename U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  RefPtr& operator=(const RefPtr<U>& o) noexcept { reset(o.ptr_); return *this; }

  RefPtr(T* ptr) noexcept : ptr_(ptr) { incRefIfNotNull(ptr); }
  RefPtr& operator=(T* ptr) noexcept { reset(ptr); return *this; }

  RefPtr(nullptr_t) noexcept : ptr_(nullptr) {}
  RefPtr& operator=(nullptr_t) noexcept { reset(); return *this; }

  [[nodiscard]] T* leakPtr() noexcept { return exchange(ptr_, nullptr); }

  void reset(T* new_ptr = nullptr) noexcept {
    incRefIfNotNull(new_ptr);
    decRefIfNotNull(exchange(ptr_, new_ptr));
  }

  ALWAYS_INLINE T* get() const noexcept { return ptr_; }

  ALWAYS_INLINE T& operator*() const noexcept { ASSERT(ptr_); return *ptr_; }
  ALWAYS_INLINE T* operator->() const noexcept { ASSERT(ptr_); return ptr_; }

  ALWAYS_INLINE bool operator!() const noexcept { return !ptr_; }
  ALWAYS_INLINE explicit operator bool() const noexcept { return ptr_ != nullptr; }

  friend void swap(RefPtr& lhs, RefPtr& rhs) noexcept { swap(lhs.ptr_, rhs.ptr_); }

 private:
  friend RefPtr adoptRc<T>(T* ptr) noexcept;

  enum AdoptTag { Adopt };

  T* ptr_;

  RefPtr(T* ptr, AdoptTag) noexcept : ptr_(ptr) {}

  ALWAYS_INLINE void incRefIfNotNull(T* ptr) noexcept {
    if (ptr)
      ptr->incRef();
  }

  ALWAYS_INLINE void decRefIfNotNull(T* ptr) noexcept {
    if (ptr)
      ptr->decRef();
  }

  RefPtr& assignMove(T* new_ptr) {
    decRefIfNotNull(exchange(ptr_, new_ptr));
    return *this;
  }

  template<typename U>
  friend bool operator==(const RefPtr& a, const RefPtr<U>& b) noexcept { return a.get() == b.get(); }
  template<typename U>
  friend bool operator==(const RefPtr& a, U* b) noexcept { return a.get() == b; }
  template<typename U>
  friend bool operator==(T* a, const RefPtr<U>& b) noexcept { return a == b.get(); }
  template<typename U>
  friend bool operator!=(const RefPtr& a, const RefPtr<U>& b) noexcept { return a.get() != b.get(); }
  template<typename U>
  friend bool operator!=(const RefPtr& a, U* b) noexcept { return a.get() != b; }
  template<typename U>
  friend bool operator!=(T* a, const RefPtr<U>& b) noexcept { return a != b.get(); }

  friend bool operator==(const RefPtr& a, nullptr_t) noexcept { return !a; }
  friend bool operator!=(const RefPtr& a, nullptr_t) noexcept { return !!a; }

  friend bool operator==(nullptr_t, const RefPtr& b) noexcept { return !b; }
  friend bool operator!=(nullptr_t, const RefPtr& b) noexcept { return !!b; }
};

template<typename T>
struct TIsZeroConstructibleTmpl<RefPtr<T>> : TTrue {};
template<typename T>
struct TIsTriviallyRelocatableTmpl<RefPtr<T>> : TTrue {};
template<typename T>
struct TIsTriviallyEqualityComparableTmpl<RefPtr<T>> : TTrue {};

template<typename T>
inline RefPtr<T> adoptRc(T* ptr) noexcept {
  adoptedByRc(ptr);
  return RefPtr<T>(ptr, RefPtr<T>::Adopt);
}

} // namespace stp

#endif // STP_BASE_MEMORY_REFPTR_H_
