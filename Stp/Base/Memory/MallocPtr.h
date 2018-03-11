// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_MALLOCPTR_H_
#define STP_BASE_MEMORY_MALLOCPTR_H_

#include "Base/Debug/Assert.h"
#include "Base/Memory/Allocate.h"
#include "Base/Type/Variable.h"

namespace stp {

template<class T>
class MallocPtr {
 public:
  static_assert(!TIsArray<T>, "C arrays disallowed, use List class instead");

  typedef T ElementType;

  MallocPtr() = default;
  ~MallocPtr() { if (ptr_) ::free(ptr_); }

  DISALLOW_COPY_AND_ASSIGN(MallocPtr);

  template<class U, TEnableIf<!TIsArray<U> && TIsConvertibleTo<U*, T*>>* = nullptr>
  MallocPtr(MallocPtr<U>&& u) : ptr_(u.leakPtr()) {}
  template<class U, TEnableIf<!TIsArray<U> && TIsConvertibleTo<U*, T*>>* = nullptr>
  MallocPtr& operator=(MallocPtr<U>&& u) { Reset(u.leakPtr()); return *this; }

  MallocPtr(nullptr_t) {}
  MallocPtr& operator=(nullptr_t) { reset(); return *this; }

  explicit MallocPtr(T* ptr) : ptr_(ptr) { ASSERT(ptr_ != nullptr); }
  [[nodiscard]] T* leakPtr() { return exchange(ptr_, nullptr); }

  void reset(T* new_ptr = nullptr) {
    T* tmp = exchange(ptr_, new_ptr);
    if (tmp)
      ::free(tmp);
  }

  T& operator*() const { ASSERT(ptr_); return *ptr_; }
  T* operator->() const { ASSERT(ptr_); return ptr_; }

  ALWAYS_INLINE T* get() const { return ptr_; }

  explicit operator bool() const { return ptr_ != nullptr; }

  static MallocPtr create(int size_in_bytes);
  static MallocPtr tryCreate(int size_in_bytes);

  friend void swap(MallocPtr& l, MallocPtr& r) { swap(l.ptr_, r.ptr_); }

  friend bool operator==(const MallocPtr& l, const MallocPtr& r) { return l.ptr_ == r.ptr_; }
  friend bool operator!=(const MallocPtr& l, const MallocPtr& r) { return l.ptr_ != r.ptr_; }

  friend bool operator==(const MallocPtr& l, T* r) { return l.ptr_ == r; }
  friend bool operator!=(const MallocPtr& l, T* r) { return l.ptr_ != r; }
  friend bool operator==(T* l, const MallocPtr& r) { return l == r.ptr_; }
  friend bool operator!=(T* l, const MallocPtr& r) { return l != r.ptr_; }

 private:
  T* ptr_ = nullptr;
};

template<class T> bool operator==(const MallocPtr<T>&, nullptr_t) = delete;
template<class T> bool operator!=(const MallocPtr<T>&, nullptr_t) = delete;
template<class T> bool operator==(nullptr_t, const MallocPtr<T>&) = delete;
template<class T> bool operator!=(nullptr_t, const MallocPtr<T>&) = delete;

template<typename T>
inline MallocPtr<T> MallocPtr<T>::create(int size_in_bytes) {
  return (T*)allocateMemory(size_in_bytes);
}

template<typename T>
inline MallocPtr<T> MallocPtr<T>::tryCreate(int size_in_bytes) {
  return (T*)tryAllocateMemory(size_in_bytes);
}

template<typename T>
struct TIsZeroConstructibleTmpl<MallocPtr<T>> : TTrue {};
template<typename T>
struct TIsTriviallyRelocatableTmpl<MallocPtr<T>> : TTrue {};
template<typename T>
struct TIsTriviallyEqualityComparableTmpl<MallocPtr<T>> : TTrue {};

// Helper to transfer ownership of a raw pointer to a MallocPtr<T>.
template<typename T>
inline MallocPtr<T> makeMallocPtr(T* ptr) {
  return MallocPtr<T>(ptr);
}

} // namespace stp

#endif // STP_BASE_MEMORY_MALLOCPTR_H_
