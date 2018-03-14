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
  DISALLOW_COPY_AND_ASSIGN(Own);
 public:
  static_assert(!TIsVoid<T>, "void type");
  static_assert(!TIsArray<T>, "C arrays disallowed, use List class instead");
  static constexpr bool IsZeroConstructible = true;
  static constexpr bool IsTriviallyRelocatable = true;
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
    exchange(*this, move(o)); return *this;
  }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  Own& operator=(Own<U>&& o) noexcept {
    exchange(*this, move(o)); return *this;
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

  template<class... TArgs>
  static Own create(TArgs&&... args);

  friend void swap(Own& l, Own& r) noexcept { swap(l.ptr_, r.ptr_); }

 private:
  T* ptr_;
};

template<class T> template<class... TArgs>
inline Own<T> Own<T>::create(TArgs&&... args) {
  return Own(*new T(forward<TArgs>(args)...));
}

// Helper to transfer ownership of a raw pointer to a Own<T>.
template<class T> inline Own<T> makeOwn(T& object) noexcept {
  return Own<T>(object);
}

template<class T> inline Borrow<T> borrow(const Own<T>& x) noexcept {
  return Borrow<T>(x.get());
}
template<class T> Borrow<T> borrow(Own<T>&& x) = delete;

} // namespace stp

#endif // STP_BASE_MEMORY_OWN_H_
