// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_WEAKPTR_H_
#define STP_BASE_MEMORY_WEAKPTR_H_

#include "Base/Memory/RefCountedThreadSafe.h"
#include "Base/Thread/ThreadChecker.h"

namespace stp {

class BaseApplicationPart;

template<typename T> class SupportsWeakPtr;
template<typename T> class WeakPtr;

namespace detail {

class BASE_EXPORT WeakReference {
 public:
  // Although Flag is bound to a specific thread, it may be
  // deleted from another via WeakPtr::~WeakPtr().
  class BASE_EXPORT Flag : public RefCountedThreadSafe<Flag> {
   public:
    static RefPtr<Flag> create() { return adoptRc(new Flag()); }

    // A sentinel object used by WeakReference objects that don't point to
    // a valid Flag, either because they're default constructed or because
    // they have been invalidated. This can be used like any other Flag object,
    // but it is invalidated already from the start, and its refcount will never
    // reach zero.
    static Flag* Null;

    void invalidate() {
      #if ASSERT_IS_ON
      if (this == Null) {
        // The null flag does not participate in the sequence checks below.
        // Since its state never changes, it can be accessed from any thread.
        ASSERT(valid_ == 0);
        return;
      }
      #endif
      // The flag being invalidated with a single ref implies that there are no
      // weak pointers in existence. Allow deletion on other thread in this case.
      ASSERT(thread_checker_.calledOnValidThread() || hasOneRef(),
             "WeakPtrs must be invalidated on the same sequenced thread");
      valid_ = 0;
    }

    uintptr_t getValidMask() const {
      #if ASSERT_IS_ON
      if (this == Null) {
        ASSERT(valid_ == 0);
        return 0;
      }
      #endif
      ASSERT(thread_checker_.calledOnValidThread(),
             "WeakPtrs must be checked on the same sequenced thread");
      return valid_;
    }

   private:
    friend class stp::BaseApplicationPart;
    friend class RefCountedThreadSafe<Flag>;

    uintptr_t valid_;
    #if ASSERT_IS_ON
    ThreadChecker thread_checker_;
    #endif

    explicit Flag(uintptr_t valid = ~static_cast<uintptr_t>(0))
        : valid_(valid) {
      #if ASSERT_IS_ON
      // Flags only become bound when checked for validity, or invalidated,
      // so that we can check that later validity/invalidation operations on
      // the same Flag take place on the same sequenced thread.
      thread_checker_.DetachFromThread();
      #endif
    }
    ~Flag() {}

    static void classInit();
  };

  WeakReference() {}
  WeakReference(RefPtr<Flag> flag) : flag_(move(flag)) {}
  ~WeakReference() {}

  WeakReference(WeakReference&& other)
      : flag_(move(other.flag_)) { other.flag_ = Flag::Null; }

  WeakReference& operator=(WeakReference&& other) {
    if (this != &other) {
      flag_ = move(other.flag_);
      other.flag_ = Flag::Null;
    }
    return *this;
  }

  WeakReference(const WeakReference& other) = default;
  WeakReference& operator=(const WeakReference& other) = default;

  uintptr_t getValidMask() const { return flag_->getValidMask(); }

 private:
  RefPtr<const Flag> flag_;
};

class BASE_EXPORT WeakReferenceOwner {
 public:
  WeakReferenceOwner() : flag_(WeakReference::Flag::Null) { }
  ~WeakReferenceOwner() { Invalidate(); }

  WeakReference GetRef() const {
    // If we hold the last reference to the Flag then create a new one.
    if (!hasRefs())
      flag_ = WeakReference::Flag::create();

    return WeakReference(flag_);
  }

  bool hasRefs() const {
    return flag_ != WeakReference::Flag::Null && !flag_->hasOneRef();
  }

  void Invalidate() {
    if (flag_ != WeakReference::Flag::Null) {
      flag_->invalidate();
      flag_ = WeakReference::Flag::Null;
    }
  }

 private:
  mutable RefPtr<WeakReference::Flag> flag_;
};

// This class simplifies the implementation of WeakPtr's type conversion
// constructor by avoiding the need for a public accessor for ref_.  A
// WeakPtr<T> cannot access the private members of WeakPtr<U>, so this
// base class gives us a way to access ref_ in a protected fashion.
class BASE_EXPORT WeakPtrBase {
 public:
  WeakPtrBase() : ptr_(0) {}
  ~WeakPtrBase() {}

  WeakPtrBase(const WeakPtrBase& other) = default;
  WeakPtrBase(WeakPtrBase&& other) = default;
  WeakPtrBase& operator=(const WeakPtrBase& other) = default;
  WeakPtrBase& operator=(WeakPtrBase&& other) = default;

 protected:
  WeakPtrBase(const WeakReference& ref, uintptr_t ptr)
      : ref_(ref), ptr_(ptr) {}

  WeakReference ref_;

  // This pointer is only valid when ref_.getValidMask() is not zero.
  // Otherwise, its value is undefined (as opposed to nullptr).
  uintptr_t ptr_;
};

// This class provides a common implementation of common functions that would
// otherwise get instantiated separately for each distinct instantiation of
// SupportsWeakPtr<>.
class SupportsWeakPtrBase {
 public:
  // A safe static downcast of a WeakPtr<Base> to WeakPtr<Derived>. This
  // conversion will only compile if there is exists a Base which inherits
  // from SupportsWeakPtr<Base>. See asWeakPtr() below for a helper
  // function that makes calling this easier.
  template<typename TDerived>
  static WeakPtr<TDerived> staticAsWeakPtr(TDerived* t) {
    static_assert(
        TIsBaseOf<detail::SupportsWeakPtrBase, TDerived>,
        "asWeakPtr argument must inherit from SupportsWeakPtr");
    return asWeakPtrImpl<TDerived>(t, *t);
  }

 private:
  // This template function uses type inference to find a Base of Derived
  // which is an instance of SupportsWeakPtr<Base>. We can then safely
  // static_cast the Base* to a Derived*.
  template<typename TDerived, typename TBase>
  static WeakPtr<TDerived> asWeakPtrImpl(
      TDerived* t, const SupportsWeakPtr<TBase>&) {
    WeakPtr<TBase> ptr = t->TBase::asWeakPtr();
    return WeakPtr<TDerived>(ptr.ref_, static_cast<TDerived*>(ptr.ptr_));
  }
};

} // namespace detail

template<typename T> class WeakPtrFactory;

template<typename T>
class WeakPtr : public detail::WeakPtrBase {
 public:
  WeakPtr() {}

  WeakPtr(nullptr_t) {}

  // Allow conversion from U to T provided U "is a" T. Note that this
  // is separate from the (implicit) copy and move constructors.
  template<typename U>
  WeakPtr(const WeakPtr<U>& other) : WeakPtrBase(other) {
    // Need to cast from U* to T* to do pointer adjustment in case of multiple
    // inheritance. This also enforces the "U is a T" rule.
    T* t = reinterpret_cast<U*>(other.ptr_);
    ptr_ = reinterpret_cast<uintptr_t>(t);
  }

  template<typename U>
  WeakPtr(WeakPtr<U>&& other) : WeakPtrBase(move(other)) {
    // Need to cast from U* to T* to do pointer adjustment in case of multiple
    // inheritance. This also enforces the "U is a T" rule.
    T* t = reinterpret_cast<U*>(other.ptr_);
    ptr_ = reinterpret_cast<uintptr_t>(t);
  }

  T* get() const {
    // Intentionally bitwise and; see command on Flag::IsValid(). This provides
    // a fast way of conditionally retrieving the pointer, and conveniently sets
    // EFLAGS for any null-check performed by the caller.
    return reinterpret_cast<T*>(ref_.getValidMask() & ptr_);
  }

  T& operator*() const {
    ASSERT(get() != nullptr);
    return *get();
  }
  T* operator->() const {
    ASSERT(get() != nullptr);
    return get();
  }

  void reset() {
    ref_ = detail::WeakReference();
    ptr_ = 0;
  }

  // Allow conditionals to test validity, e.g. if (weak_ptr) {...};
  explicit operator bool() const { return get() != nullptr; }

 private:
  friend class detail::SupportsWeakPtrBase;
  template<typename U> friend class WeakPtr;
  friend class SupportsWeakPtr<T>;
  friend class WeakPtrFactory<T>;

  WeakPtr(const detail::WeakReference& ref, T* ptr)
      : WeakPtrBase(ref, reinterpret_cast<uintptr_t>(ptr)) {}

  // Allow callers to compare WeakPtrs against nullptr to test validity.
  friend bool operator==(const WeakPtr& weak_ptr, nullptr_t) {
    return weak_ptr.get() == nullptr;
  }
  friend bool operator!=(const WeakPtr& weak_ptr, nullptr_t) {
    return !(weak_ptr == nullptr);
  }
  friend bool operator==(nullptr_t, const WeakPtr& weak_ptr) {
    return weak_ptr == nullptr;
  }
  friend bool operator!=(nullptr_t, const WeakPtr& weak_ptr) {
    return weak_ptr != nullptr;
  }
};

template<class T>
class WeakPtrFactory {
 public:
  explicit WeakPtrFactory(T* ptr)
      : ptr_(reinterpret_cast<uintptr_t>(ptr)) {}

  ~WeakPtrFactory() { ptr_ = 0; }

  WeakPtr<T> getWeakPtr() {
    ASSERT(ptr_);
    return WeakPtr<T>(
        weak_reference_owner_.GetRef(), reinterpret_cast<T*>(ptr_));
  }

  // Call this method to invalidate all existing weak pointers.
  void invalidateWeakPtrs() {
    ASSERT(ptr_);
    weak_reference_owner_.Invalidate();
  }

  // Call this method to determine if any weak pointers exist.
  bool hasWeakPtrs() const {
    ASSERT(ptr_);
    return weak_reference_owner_.hasRefs();
  }

 private:
  detail::WeakReferenceOwner weak_reference_owner_;
  uintptr_t ptr_;
  DISALLOW_COPY_AND_ASSIGN(WeakPtrFactory);
};

template<class T>
class SupportsWeakPtr : public detail::SupportsWeakPtrBase {
 public:
  SupportsWeakPtr() {}

  WeakPtr<T> asWeakPtr() {
    return WeakPtr<T>(weak_reference_owner_.GetRef(), static_cast<T*>(this));
  }

 protected:
  ~SupportsWeakPtr() {}

 private:
  detail::WeakReferenceOwner weak_reference_owner_;
  DISALLOW_COPY_AND_ASSIGN(SupportsWeakPtr);
};

template<typename TDerived>
WeakPtr<TDerived> asWeakPtr(TDerived* t) {
  return detail::SupportsWeakPtrBase::staticAsWeakPtr<TDerived>(t);
}

} // namespace stp

#endif // STP_BASE_MEMORY_WEAKPTR_H_
