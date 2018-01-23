// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CALL_DELEGATE_H_
#define STP_BASE_CALL_DELEGATE_H_

#include "Base/Call/DelegateFwd.h"
#include "Base/Compiler/Cpu.h"
#include "Base/Debug/Assert.h"
#include "Base/Type/HashableFwd.h"
#include "Base/Type/Variable.h"

namespace stp {

namespace delegate_impl {

typedef void (*GenericFunction)();

#if COMPILER(MSVC)
class __single_inheritance GenericClass;
class GenericClass {};

// Define a generic class that uses virtual inheritance.
// It has a trival member function that returns the value of the 'this' pointer.
struct GenericVirtualClass : virtual public GenericClass {
  typedef GenericVirtualClass* (GenericVirtualClass::*ProbeFunction)();
  GenericVirtualClass* GetThis() { return this; }
};
#else
class GenericClass;
#endif

typedef void (GenericClass::*GenericMethod)();
typedef void (*GenericStaticMethod)(GenericClass*);

template<int N>
struct SimplifyMethod {
  template<class X, class XMethod, typename GMethod>
  static GenericClass* Convert(XMethod method, X* that, GMethod& gmethod) {
    static_assert(N < 0, "unsupported member function pointer");
    return nullptr;
  }
};

#if COMPILER(GCC)

template<>
struct SimplifyMethod<sizeof(GenericMethod)> {
  template<class X, class XMethod, typename GSMethod>
  static ALWAYS_INLINE GenericClass*
  Convert(XMethod method, const X* thatc, GSMethod& gsmethod) {
    struct GnuMemberFunction {
      union {
        // If even, it's a pointer to the function.
        GSMethod function;
        // If odd, it's the byte offset into the vtable.
        long vtable_offset;
      };
      long this_delta;
    };

    X* that = const_cast<X*>(thatc);
    GnuMemberFunction mf = bit_cast<GnuMemberFunction>(method);
    #if CPU(X86_FAMILY)
    auto* object = reinterpret_cast<GenericClass*>(
        reinterpret_cast<char*>(that) + mf.this_delta);

    if (mf.vtable_offset & 1) {
      char* vtable_base = *reinterpret_cast<char**>(object);
      gsmethod = *reinterpret_cast<GSMethod*>(vtable_base + mf.vtable_offset - 1);
    } else {
      gsmethod = mf.function;
    }
    return object;
    #elif CPU(ARM_FAMILY)
    auto* object = reinterpret_cast<GenericClass*>(
        reinterpret_cast<char*>(that) + (mf.this_delta >> 1));

    if (mf.this_delta & 1) {
      char* vtable_base = *reinterpret_cast<char**>(object);
      gsmethod =  *reinterpret_cast<GSMethod*>(vtable_base + mf.vtable_offset);
    } else {
      gsmethod = mf.function;
    }
    return object;
    #else
    # error "cpu not supported"
    #endif // CPU(*)
  }
};

#elif COMPILER(MSVC)

// MSVC has different pointer sizes depending on type of class.
// - single inheritance
// - multiple inheritance
// - virtual inheritance
// - unknown inheritance

template<>
struct SimplifyMethod<sizeof(GenericStaticMethod)> {
  template<class X, class XMethod, typename GSMethod>
  static ALWAYS_INLINE GenericClass*
  Convert(XMethod method, const X* thatc, GSMethod& gsmethod) {
    gsmethod = bit_cast<GSMethod>(method);
    X* that = const_cast<X*>(thatc);
    return reinterpret_cast<GenericClass*>(that);
  }
};

template<>
struct SimplifyMethod<sizeof(GenericStaticMethod) + sizeof(int)> {
  template<class X, class XMethod, typename GSMethod>
  static ALWAYS_INLINE GenericClass*
  Convert(XMethod method, const X* thatc, GSMethod& gsmethod) {
    struct MSVCExtendedMemberFunction {
      GSMethod function;
      int this_delta;
    };

    MSVCExtendedMemberFunction mf = bit_cast<MSVCExtendedMemberFunction>(method);

    gsmethod = mf.function;
    X* that = const_cast<X*>(thatc);
    return reinterpret_cast<GenericClass*>(reinterpret_cast<char*>(that) + mf.this_delta);
  }
};

template<>
struct SimplifyMethod<sizeof(GenericStaticMethod) + 2 * sizeof(int)> {
  template<class X, class XMethod, typename GSMethod>
  static ALWAYS_INLINE GenericClass*
  Convert(XMethod method, const X* thatc, GSMethod& gsmethod) {
    struct MSVCVirtualMemberFunction {
      GSMethod function;
      int this_delta;
      int vtable_index;
    };

    MSVCVirtualMemberFunction mf = bit_cast<MSVCVirtualMemberFunction>(method);
    int this_delta = mf.this_delta;
    gsmethod = mf.function;

    X* that = const_cast<X*>(thatc);
    static const int VtorDisplacement = sizeof(void*);
    if (mf.vtable_index != 0) {
      // Resolve VTable.
      const int* vtable = *reinterpret_cast<const int* const*>(
          reinterpret_cast<const char*>(that) + VtorDisplacement);

      // |vtable_index| tells us where in the table we should be looking.
      this_delta += VtorDisplacement + *reinterpret_cast<const int*>(
          reinterpret_cast<const char*>(vtable) + mf.vtable_index);
    }
    return reinterpret_cast<GenericClass*>(reinterpret_cast<char*>(that) + this_delta);
  }
};

struct MSVCUnknownMemberFunction {
  GenericStaticMethod function;
  int this_delta;
  int vtor_displacement;
  int vtable_index;
};

template<>
struct SimplifyMethod<sizeof(MSVCUnknownMemberFunction)> {
  template<class X, class XMethod, typename GSMethod>
  static ALWAYS_INLINE GenericClass*
  Convert(XMethod method, const X* thatc, GSMethod& gsmethod) {
    MSVCUnknownMemberFunction mf = bit_cast<MSVCUnknownMemberFunction>(method);

    GenericClass* gobject = reinterpret_cast<GenericClass*>(thatc);
    int this_delta = mf.this_delta;
    gsmethod = bit_cast<GSMethod>(mf.function);

    X* that = const_cast<X*>(thatc);
    // If index is zero, fallback to extended member function.
    if (mf.vtable_index != 0) {
      // Resolve VTable.
      const int* vtable = *reinterpret_cast<const int* const*>(
          reinterpret_cast<const char*>(that) + mf.vtor_displacement);

      // |vtable_index| tells us where in the table we should be looking.
      this_delta += mf.vtor_displacement + *reinterpret_cast<const int*>(
          reinterpret_cast<const char*>(vtable) + mf.vtable_index);
    }
    return reinterpret_cast<GenericClass*>(reinterpret_cast<char*>(gobject) + this_delta);
  }
};

#else
# error "compiler not supported"
#endif // COMPILER(*)

BASE_EXPORT void FormatDelegate(TextWriter& out, const StringSpan& opts, void* ptr);

} // namespace delegate_impl

template<typename R, typename... TArgs>
class Delegate<R(TArgs...)> {
 public:
  Delegate()
      : gobject_(nullptr), gmethod_(nullptr) {}

  Delegate(nullptr_t)
      : gobject_(nullptr), gmethod_(nullptr) {}

  Delegate(const Delegate& other)
      : gobject_(other.gobject_),
        gmethod_(other.gmethod_) {}

  template<class X>
  Delegate(R (X::*method)(TArgs...), X* object) {
    gobject_ = delegate_impl::SimplifyMethod<sizeof(method)>::Convert(method, object, gmethod_);
  }

  R operator()(TArgs... args) const {
    ASSERT(gmethod_ != nullptr);
    return (*gmethod_)(gobject_, Forward<TArgs>(args)...);
  }

  Delegate& operator=(const Delegate& other) {
    gobject_ = other.gobject_;
    gmethod_ = other.gmethod_;
    return *this;
  }

  explicit operator bool() const { return gmethod_ != nullptr; }
  bool IsNull() const { return gmethod_ == nullptr; }

  friend HashCode Hash(const Delegate& x) { return Hash(x.gmethod_); }
  friend void Format(TextWriter& out, const Delegate& x, const StringSpan& opts) {
    delegate_impl::FormatDelegate(out, opts, reinterpret_cast<void*>(x.gmethod_));
  }

 private:
  delegate_impl::GenericClass* gobject_;
  R (*gmethod_)(delegate_impl::GenericClass*, TArgs...);
};

template<typename TClass, typename TThis, typename TResult, typename... TArgs>
inline Delegate<TResult(TArgs...)> MakeDelegate(TResult(TClass::*method)(TArgs...), TThis* that) {
  TClass* thatx = that;
  return Delegate<TResult(TArgs...)>(method, thatx);
}

} // namespace stp

#endif // STP_BASE_CALL_DELEGATE_H_
