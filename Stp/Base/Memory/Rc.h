// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_RC_H_
#define STP_BASE_MEMORY_RC_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Variable.h"

namespace stp {

template<class T>
class Rc;

template<class T>
Rc<T> adoptRc(T& object);

ALWAYS_INLINE void adoptedByRc(const void*) {}

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

  Rc(Rc&& o) : ptr_(&o.leakRef()) {}
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  Rc(Rc<U>&& o) : ptr_(&o.leakRef()) {}

  Rc& operator=(Rc&& o) {
    exchange(*this, move(o)); return *this;
  }
  template<class U, TEnableIf<TIsConvertibleTo<U*, T*>>* = nullptr>
  Rc& operator=(Rc<U>&& o) {
    exchange(*this, move(o)); return *this;
  }

  Rc(T& object) : ptr_(&object) { object.incRef(); }

  Rc& operator=(T& object) {
    Rc copy = object; swap(*this, copy); return *this;
  }

  Rc copyRef() && = delete;
  [[nodiscard]] Rc copyRef() const & { return Rc(*ptr_); }

  [[nodiscard]] T& leakRef() {
    ASSERT(ptr_);
    T& result = *exchange(ptr_, nullptr);
    #if SANITIZER(ADDRESS)
    __asan_poison_memory_region(this, sizeof(*this));
    #endif
    return result;
  }

  T& get() const { ASSERT(ptr_); return *ptr_; }
  operator T&() const { ASSERT(ptr_); return *ptr_; }

  T& operator*() const { ASSERT(ptr_); return *ptr_; }
  T* operator->() const { ASSERT(ptr_); return ptr_; }

  friend void swap(Rc& lhs, Rc& rhs) { swap(lhs.ptr_, rhs.ptr_); }

  // FIXME
//  Rc(NullableConstructTag) : ptr_(nullptr) {}
//  bool isNullForNullable() const { return ptr_ == nullptr; }
//  T* getForNullable() const { return ptr_; }
//  Rc takeForNullable() { return leakRef(); }

 private:
  friend Rc adoptRc<T>(T& ptr);

  enum AdoptTag { Adopt };

  T* ptr_;

  Rc(T& object, AdoptTag) : ptr_(&object) {}
};

template<class T>
struct TIsTriviallyRelocatableTmpl<Rc<T>> : TTrue {};

template<class T>
inline Rc<T> adoptRc(T& object) {
  adoptedByRc(&object);
  return Rc<T>(object, Rc<T>::Adopt);
}

template<class T>
inline Rc<T> makeRc(T& object) {
  return Rc<T>(object);
}

template<class T>
inline Borrow<T> borrow(const Rc<T>& x) {
  return Borrow<T>(x.get());
}

} // namespace stp

#endif // STP_BASE_MEMORY_RC_H_
