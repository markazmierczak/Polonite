// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_OWN_H_
#define STP_BASE_MEMORY_OWN_H_

#include "Base/Debug/Assert.h"
#include "Base/Memory/Allocate.h"
#include "Base/Type/Variable.h"

namespace stp {

template<class T>
class Own {
 public:
  static_assert(!TIsVoid<T>, "void type");
  static_assert(!TIsArray<T>, "C arrays disallowed, use List class instead");

  typedef T ElementType;

  ~Own() {
    #if SANITIZER(ADDRESS)
    if (__asan_address_is_poisoned(this))
      __asan_unpoison_memory_region(this, sizeof(*this));
    #endif
    if (ptr_)
      delete ptr_;
  }

  Own(Own&& o) noexcept : ptr_(&o.leakRef()) {}

  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  Own(Own<U>&& o) noexcept : ptr_(&o.leakRef()) {}

  Own& operator=(Own&& o) noexcept {
    Own moved = move(o); swap(*this, moved); return *this;
  }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  Own& operator=(Own<U>&& o) noexcept {
    Own moved = move(o); swap(*this, moved); return *this;
  }

  explicit Own(T& object) noexcept : ptr_(&object) { ASSERT(ptr_); }

  [[nodiscard]] T& leakRef() noexcept {
    T& result = *exchange(ptr_, nullptr);
    #if SANITIZER(ADDRESS)
    __asan_poison_memory_region(this, sizeof(*this));
    #endif
    return result;
  }

  T& operator*() const noexcept { ASSERT(ptr_); return *ptr_; }
  T* operator->() const noexcept { ASSERT(ptr_); return ptr_; }

  T& get() const noexcept { ASSERT(ptr_); return *ptr_; }
  operator T&() const noexcept { ASSERT(ptr_); return *ptr_; }

  template<typename... TArgs>
  static Own create(TArgs&&... args);

  friend void swap(Own& l, Own& r) noexcept { swap(l.ptr_, r.ptr_); }

 private:
  T* ptr_;

  DISALLOW_COPY_AND_ASSIGN(Own);
};

template<typename T>
template<typename... TArgs>
inline Own<T> Own<T>::create(TArgs&&... args) {
  return Own(*new T(forward<TArgs>(args)...));
}

template<typename T>
struct TIsZeroConstructibleTmpl<Own<T>> : TTrue {};
template<typename T>
struct TIsTriviallyRelocatableTmpl<Own<T>> : TTrue {};
template<typename T>
struct TIsTriviallyEqualityComparableTmpl<Own<T>> : TTrue {};

// Helper to transfer ownership of a raw pointer to a Own<T>.
template<typename T>
inline Own<T> makeOwn(T& object) {
  return Own<T>(object);
}

} // namespace stp

#endif // STP_BASE_MEMORY_OWN_H_
