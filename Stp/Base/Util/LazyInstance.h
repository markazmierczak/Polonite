// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_UTIL_LAZYINSTANCE_H_
#define STP_BASE_UTIL_LAZYINSTANCE_H_

#include "Base/Compiler/Lsan.h"
#include "Base/Debug/Assert.h"
#include "Base/Mem/Allocate.h"
#include "Base/Thread/AtomicOps.h"
#include "Base/Type/Variable.h"

namespace stp {

namespace detail {

template<typename T>
struct ErrorLazyInstanceTraits {};

template<typename T>
struct DefaultLazyInstanceTraits {
  static constexpr bool RegisterOnExit = true;

  static T* New(void* instance) {
    ASSERT((reinterpret_cast<uintptr_t>(instance) & (alignof(T) - 1)) == 0);
    // Use placement new to initialize our instance in our preallocated space.
    // The parenthesis is very important here to force POD type initialization.
    return new (instance) T();
  }
  static void Delete(T* instance) {
    // Explicitly call the destructor.
    instance->~T();
  }
};

template<typename T>
struct LeakyLazyInstanceTraits {
  static constexpr bool RegisterOnExit = false;

  static T* New(void* instance) {
    ANNOTATE_SCOPED_MEMORY_LEAK;
    return DefaultLazyInstanceTraits<T>::New(instance);
  }
  static void Delete(T* instance) {
  }
};

constexpr subtle::AtomicWord LazyInstanceStateCreating = 1;

// Check if instance needs to be created. If so return true otherwise
// if another thread has beat us, wait for instance to be created and
// return false.
BASE_EXPORT bool NeedsLazyInstance(subtle::AtomicWord* state);

// After creating an instance, call this to register the dtor to be called
// at program exit and to update the atomic state to hold the |new_instance|
BASE_EXPORT void CompleteLazyInstance(
    subtle::AtomicWord* state, subtle::AtomicWord new_instance,
    void* lazy_instance, void (*dtor)(void*));

} // namespace detail

#define LAZY_INSTANCE_INITIALIZER {0}

template<typename T, typename Traits = detail::ErrorLazyInstanceTraits<T> >
class LazyInstance {
 public:
  typedef LazyInstance<T, detail::DefaultLazyInstanceTraits<T>> DestroyAtExit;
  typedef LazyInstance<T, detail::LeakyLazyInstanceTraits<T>> LeakAtExit;

  T* Pointer();

  bool operator==(T* p);

  T* operator->() { return Pointer(); }
  T& operator*() { return *Pointer(); }

  // Effectively private: member data is only public to allow the linker to
  // statically initialize it and to maintain a POD class.
  // DO NOT USE FROM OUTSIDE THIS CLASS.

  // Preallocated space for the T instance.
  AlignedByteArray<sizeof(T), alignof(T)> private_buf_;

  subtle::AtomicWord private_instance_;

 private:
  T* GetInstance() {
    return reinterpret_cast<T*>(subtle::NoBarrier_Load(&private_instance_));
  }

  // Adapter function for use with AtExit.
  // This should be called single threaded, so don't synchronize across threads.
  static void OnExit(void* lazy_instance) {
    auto* me = reinterpret_cast<LazyInstance<T, Traits>*>(lazy_instance);
    Traits::Delete(me->GetInstance());
    subtle::NoBarrier_Store(&me->private_instance_, 0);
  }
};

template<typename T, typename Traits>
inline T* LazyInstance<T, Traits>::Pointer() {
  // If any bit in the created mask is true, the instance has already been fully constructed.
  static constexpr subtle::AtomicWord LazyInstanceCreatedMask = ~detail::LazyInstanceStateCreating;
  subtle::AtomicWord value = subtle::Acquire_Load(&private_instance_);
  if (!(value & LazyInstanceCreatedMask) && detail::NeedsLazyInstance(&private_instance_)) {
    constexpr bool NeedsAtExit = Traits::RegisterOnExit && !TIsTriviallyDestructible<T>;
    value = reinterpret_cast<subtle::AtomicWord>(Traits::New(private_buf_.bytes));
    detail::CompleteLazyInstance(&private_instance_, value, this, NeedsAtExit ? OnExit : nullptr);
  }
  return GetInstance();
}

template<typename T, typename Traits>
inline bool LazyInstance<T, Traits>::operator==(T* p) {
  switch (subtle::NoBarrier_Load(&private_instance_)) {
    case 0:
      return p == nullptr;
    case detail::LazyInstanceStateCreating:
      return p == reinterpret_cast<T*>(private_buf_.bytes);
    default:
      return p == GetInstance();
  }
}

} // namespace stp

#endif // STP_BASE_UTIL_LAZYINSTANCE_H_
