// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_ERROR_EXCEPTIONPTR_H_
#define STP_BASE_ERROR_EXCEPTIONPTR_H_

#include "Base/Error/Exception.h"
#include "Base/Mem/AddressOf.h"
#include "Base/Type/Variable.h"

#include <cstdlib>

#if COMPILER(MSVC)
#include <vcruntime_exception.h>
#include <yvals.h> // for _CRTIMP2_PURE

_CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL __ExceptionPtrCreate(_Out_ void*);
_CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL __ExceptionPtrDestroy(_Inout_ void*);
_CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL __ExceptionPtrCopy(_Out_ void*, _In_ const void*);
_CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL __ExceptionPtrAssign(_Inout_ void*, _In_ const void*);
_CRTIMP2_PURE bool __CLRCALL_PURE_OR_CDECL __ExceptionPtrCompare(_In_ const void*, _In_ const void*);
_CRTIMP2_PURE bool __CLRCALL_PURE_OR_CDECL __ExceptionPtrToBool(_In_ const void*);
_CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL __ExceptionPtrSwap(_Inout_ void*, _Inout_ void*);
_CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL __ExceptionPtrCurrentException(_Out_ void*);
[[noreturn]]
_CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL __ExceptionPtrRethrow(_In_ const void*);
_CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL __ExceptionPtrCopyException(_Inout_ void*, _In_ const void*, _In_ const void*);

#else

#include <cxxabi.h>
# if defined(__GLIBCXX__)
# include <exception>
# endif

#endif

namespace stp {

class ExceptionPtr;

bool operator==(const ExceptionPtr& l, const ExceptionPtr& r) noexcept;

class ExceptionPtr {
 public:
  static ExceptionPtr Current() noexcept;

  ExceptionPtr() noexcept;
  ~ExceptionPtr() noexcept;

  ExceptionPtr(const ExceptionPtr& other) noexcept;
  ExceptionPtr& operator=(const ExceptionPtr& other) noexcept;

  explicit operator bool() const noexcept;

  [[noreturn]] void Rethrow();

  friend bool operator==(const ExceptionPtr& l, const ExceptionPtr& r) noexcept;
  friend bool operator!=(const ExceptionPtr& l, const ExceptionPtr& r) noexcept {
    return !operator==(l, r);
  }

 private:
  #if COMPILER(MSVC)
  # if defined(__clang__)
  # pragma clang diagnostic push
  # pragma clang diagnostic ignored "-Wunused-private-field"
  # endif
  void* ptr1_;
  void* ptr2_;
  # if defined(__clang__)
  # pragma clang diagnostic pop
  # endif
  #elif defined(__GLIBCXX__)
  std::exception_ptr impl_;
  #elif defined(_LIBCPPABI_VERSION)
  void* ptr_;
  #endif
};

template<class TException>
inline ExceptionPtr MakeExceptionPtr(TException e) noexcept {
  #if COMPILER(MSVC)
  ExceptionPtr rv = nullptr;
  const void* info = __GetExceptionInfo(e);
  if (info)
    __ExceptionPtrCopyException(&rv, AddressOf(e), info);
  return rv;
  #else
  try {
    throw e;
  } catch (...) {
    return ExceptionPtr::Current();
  }
  #endif
}

#if COMPILER(MSVC)

inline ExceptionPtr::ExceptionPtr() noexcept {
  __ExceptionPtrCreate(this);
}

inline ExceptionPtr::ExceptionPtr(nullptr_t) noexcept {
  __ExceptionPtrCreate(this);
}

inline ExceptionPtr::ExceptionPtr(const ExceptionPtr& other) noexcept {
  __ExceptionPtrCopy(this, &other);
}

inline ExceptionPtr& ExceptionPtr::operator=(const ExceptionPtr& other) noexcept {
  __ExceptionPtrAssign(this, &other);
  return *this;
}

inline ExceptionPtr& ExceptionPtr::operator=(nullptr_t) noexcept {
  ExceptionPtr dummy;
  __ExceptionPtrAssign(this, &dummy);
  return *this;
}

inline ExceptionPtr::~ExceptionPtr() noexcept {
  __ExceptionPtrDestroy(this);
}

inline ExceptionPtr::operator bool() const noexcept {
  return __ExceptionPtrToBool(this);
}

inline bool operator==(const ExceptionPtr& l, const ExceptionPtr& r) noexcept {
  return __ExceptionPtrCompare(&l, &r);
}

inline void Swap(ExceptionPtr& lhs, ExceptionPtr& rhs) noexcept {
  __ExceptionPtrSwap(&rhs, &lhs);
}

[[noreturn]] inline void ExceptionPtr::Rethrow() {
  __ExceptionPtrRethrow(this);
}

inline ExceptionPtr ExceptionPtr::Current() noexcept {
  ExceptionPtr rv;
  __ExceptionPtrCurrentException(&rv);
  return rv;
}

#elif defined(__GLIBCXX__)

inline ExceptionPtr::ExceptionPtr() noexcept {}
inline ExceptionPtr::~ExceptionPtr() noexcept {}

inline ExceptionPtr::ExceptionPtr(const ExceptionPtr& other) noexcept
    : impl_(other.impl_) {
}

inline ExceptionPtr& ExceptionPtr::operator=(const ExceptionPtr& other) noexcept {
  impl_ = other.impl_;
  return *this;
}

inline void Swap(ExceptionPtr& lhs, ExceptionPtr& rhs) noexcept {
  ExceptionPtr tmp = lhs;
  lhs = rhs;
  rhs = tmp;
}

inline ExceptionPtr::operator bool() const noexcept {
  return impl_.operator bool();
}

[[noreturn]] inline void ExceptionPtr::Rethrow() {
  std::rethrow_exception(impl_);
}

inline ExceptionPtr ExceptionPtr::Current() noexcept {
  ExceptionPtr rv;
  rv.impl_ = std::current_exception();
  return rv;
}

#elif defined(_LIBCPPABI_VERSION)

inline ExceptionPtr::ExceptionPtr() noexcept
    : ptr_(nullptr) {
}

inline ExceptionPtr::~ExceptionPtr() noexcept {
  __cxxabiv1::__cxa_decrement_exception_refcount(ptr_);
}

inline ExceptionPtr::ExceptionPtr(const ExceptionPtr& other) noexcept
    : ptr_(other.ptr_) {
  __cxxabiv1::__cxa_increment_exception_refcount(ptr_);
}

inline ExceptionPtr& ExceptionPtr::operator=(const ExceptionPtr& other) noexcept {
  if (ptr_ != other.ptr_) {
    __cxxabiv1::__cxa_increment_exception_refcount(other.ptr_);
    __cxxabiv1::__cxa_decrement_exception_refcount(ptr_);
    ptr_ = other.ptr_;
  }
  return *this;
}

inline ExceptionPtr::operator bool() const noexcept {
  return ptr_ != nullptr;
}

inline bool operator==(const ExceptionPtr& l, const ExceptionPtr& r) noexcept {
  return l.ptr_ == r.ptr_;
}

[[noreturn]] inline void ExceptionPtr::Rethrow() {
  __cxxabiv1::__cxa_rethrow_primary_exception(ptr_);
  // if p.ptr_ is NULL, above returns so we terminate
  Application::Terminate();
}

inline ExceptionPtr ExceptionPtr::Current() noexcept {
  ExceptionPtr rv;
  rv.ptr_ = __cxxabiv1::__cxa_current_primary_exception();
  return rv;
}

#else
# error "unknown exception handling"
#endif

} // namespace stp

#endif // STP_BASE_ERROR_EXCEPTIONPTR_H_
