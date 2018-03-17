// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_RC_H_
#define STP_BASE_MEMORY_RC_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Variable.h"

namespace stp {

template<class T> class Rc;
template<class T> class RcPtr;

template<class T> Rc<T> adoptRc(T& object) noexcept;
template<class T> RcPtr<T> adoptRc(T* object) noexcept;

ALWAYS_INLINE void adoptedByRc(const void*) noexcept {}

template<class T>
class Rc {
 public:
  ~Rc() {
    #if SANITIZER(ADDRESS)
    if (__asan_address_is_poisoned(this))
      __asan_unpoison_memory_region(this, sizeof(*this));
    #endif
    if (ptr_)
      ptr_->decRef();
  }

  Rc(const Rc&) = delete;
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  Rc(const Rc<U>&) = delete;

  Rc(Rc&& o) noexcept : ptr_(&o.leakRef()) {}
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  Rc(Rc<U>&& o) noexcept : ptr_(&o.leakRef()) {}

  Rc& operator=(Rc&& o) noexcept {
    exchange(*this, move(o)); return *this;
  }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  Rc& operator=(Rc<U>&& o) noexcept {
    exchange(*this, move(o)); return *this;
  }

  Rc(T& object) noexcept : ptr_(&object) { object.incRef(); }

  Rc& operator=(T& object) noexcept {
    Rc copy = object; swap(*this, copy); return *this;
  }

  Rc copyRef() && = delete;
  [[nodiscard]] Rc copyRef() const & noexcept { return Rc(*ptr_); }

  [[nodiscard]] T& leakRef() noexcept {
    ASSERT(ptr_);
    T& result = *exchange(ptr_, nullptr);
    #if SANITIZER(ADDRESS)
    __asan_poison_memory_region(this, sizeof(*this));
    #endif
    return result;
  }

  T& get() const noexcept { ASSERT(ptr_); return *ptr_; }
  operator T&() const noexcept { ASSERT(ptr_); return *ptr_; }

  T& operator*() const noexcept { ASSERT(ptr_); return *ptr_; }
  T* operator->() const noexcept { ASSERT(ptr_); return ptr_; }

  friend void swap(Rc& lhs, Rc& rhs) noexcept { swap(lhs.ptr_, rhs.ptr_); }

 private:
  friend Rc adoptRc<T>(T& ptr) noexcept;

  enum AdoptTag { Adopt };

  T* ptr_;

  Rc(T& object, AdoptTag) : ptr_(&object) {}
};

template<class T> struct TIsTriviallyRelocatableTmpl<Rc<T>> : TTrue {};

template<class T> inline Rc<T> adoptRc(T& object) noexcept {
  adoptedByRc(&object);
  return Rc<T>(object, Rc<T>::Adopt);
}

template<class T> inline Rc<T> makeRc(T& object) noexcept {
  return Rc<T>(object);
}

template<class T> inline Borrow<T> borrow(const Rc<T>& x) noexcept {
  return Borrow<T>(x.get());
}
template<class T> Borrow<T> borrow(Rc<T>&& x) = delete;

} // namespace stp

#endif // STP_BASE_MEMORY_RC_H_
