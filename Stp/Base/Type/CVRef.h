// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_CVREF_H_
#define STP_BASE_TYPE_CVREF_H_

#include "Base/Type/Basic.h"

namespace stp {

namespace detail {

template<typename T>
struct TIsLvalueReferenceHelper : TFalse {};

template<typename T>
struct TIsLvalueReferenceHelper<T&> : TTrue {};

template<typename T>
struct TIsRvalueReferenceHelper : TFalse {};

template<typename T>
struct TIsRvalueReferenceHelper<T&&> : TTrue {};

template<typename T>
struct TRemoveReferenceHelper {
  typedef T Type;
};

template<typename T>
struct TRemoveReferenceHelper<T&> {
  typedef T Type;
};

template<typename T>
struct TRemoveReferenceHelper<T&&> {
  typedef T Type;
};

struct TIsReferenceableHelper2 {
  template<typename T> static T& Test(int);
  template<typename T> static TUnspecified Test(...);
};

} // namespace detail

template<typename T>
constexpr bool TIsLvalueReference = detail::TIsLvalueReferenceHelper<T>::Value;

template<typename T>
constexpr bool TIsRvalueReference = detail::TIsRvalueReferenceHelper<T>::Value;

template<typename T>
constexpr bool TIsReference = TIsLvalueReference<T> || TIsRvalueReference<T>;

template<typename T>
using TRemoveReference = typename detail::TRemoveReferenceHelper<T>::Type;

template<typename T>
constexpr bool TIsReferenceable =
    !TsAreSame<decltype(detail::TIsReferenceableHelper2::Test<T>(0)), TUnspecified>;

namespace detail {

template<typename T, bool = TIsReferenceable<T>>
struct TAddLvalueReferenceHelper {
  typedef T Type;
};

template<typename T>
struct TAddLvalueReferenceHelper<T, true> {
  typedef T& Type;
};

template<typename T, bool = TIsReferenceable<T>>
struct TAddRvalueReferenceHelper {
  typedef T Type;
};

template<typename T>
struct TAddRvalueReferenceHelper<T, true> {
  typedef T&& Type;
};

} // namespace detail

template<typename T>
using TAddLvalueReference = typename detail::TAddLvalueReferenceHelper<T>::Type;

template<typename T>
using TAddRvalueReference = typename detail::TAddRvalueReferenceHelper<T>::Type;

template<typename T>
ALWAYS_INLINE constexpr TRemoveReference<T> && move(T&& t) noexcept {
  typedef TRemoveReference<T> U;
  return static_cast<U&&>(t);
}

template<typename T>
ALWAYS_INLINE constexpr T&& Forward(TRemoveReference<T>& t) noexcept {
  return static_cast<T&&>(t);
}

template<typename T>
ALWAYS_INLINE constexpr T&& Forward(TRemoveReference<T>&& t) noexcept {
  static_assert(!TIsLvalueReference<T>, "can not forward an rvalue as an lvalue");
  return static_cast<T&&>(t);
}

template<typename T, typename U = T>
constexpr T Exchange(T& obj, U&& new_val) noexcept {
  T old_val = move(obj);
  obj = Forward<U>(new_val);
  return old_val;
}

namespace detail {

template<typename T>
struct TIsConstHelper : TFalse {};

template<typename T>
struct TIsConstHelper<T const> : TTrue {};

template<typename T>
struct TIsVolatileHelper : TFalse {};

template<class T>
struct TIsVolatileHelper<T volatile> : TTrue {};

template<typename T>
struct TAddConstHelper {
  typedef T const Type;
};

template<typename T>
struct TAddVolatileHelper {
  typedef T volatile Type;
};

template<typename T>
struct TRemoveConstHelper {
  typedef T Type;
};

template<typename T>
struct TRemoveConstHelper<T const> {
  typedef T Type;
};

template<typename T>
struct TRemoveVolatileHelper {
  typedef T Type;
};

template<typename T>
struct TRemoveVolatileHelper<T volatile> {
  typedef T Type;
};

template<typename T>
struct TRemoveCVHelper {
  typedef typename TRemoveConstHelper<typename TRemoveVolatileHelper<T>::Type>::Type Type;
};

} // namespace detail

template<typename T>
constexpr bool TIsConst = detail::TIsConstHelper<T>::Value;

template<typename T>
constexpr bool TIsVolatile = detail::TIsVolatileHelper<T>::Value;

template<typename T>
using TAddConst = typename detail::TAddConstHelper<T>::Type;

template<typename T>
using TAddVolatile = typename detail::TAddVolatileHelper<T>::Type;

template<typename T>
using TRemoveConst = typename detail::TRemoveConstHelper<T>::Type;

template<typename T>
using TRemoveVolatile = typename detail::TRemoveVolatileHelper<T>::Type;

template<typename T>
using TRemoveCV = typename detail::TRemoveCVHelper<T>::Type;

namespace detail {

template<typename T, typename U,
         bool = TIsConst<TRemoveReference<T>>,
         bool = TIsVolatile<TRemoveReference<T>>
        >
struct TApplyCVHelper {
  typedef U Type;
};

template<typename T, typename U>
struct TApplyCVHelper<T, U, true, false> {
  typedef const U Type;
};

template<typename T, typename U>
struct TApplyCVHelper<T, U, false, true> {
  typedef volatile U Type;
};

template<typename T, typename U>
struct TApplyCVHelper<T, U, true, true> {
  typedef const volatile U Type;
};

template<typename T, typename U>
struct TApplyCVHelper<T&, U, false, false> {
  typedef U& Type;
};

template<typename T, typename U>
struct TApplyCVHelper<T&, U, true, false> {
  typedef const U& Type;
};

template<typename T, typename U>
struct TApplyCVHelper<T&, U, false, true> {
  typedef volatile U& Type;
};

template<typename T, typename U>
struct TApplyCVHelper<T&, U, true, true> {
  typedef const volatile U& Type;
};

} // namespace detail

template<typename T, typename U>
using TApplyCV = typename detail::TApplyCVHelper<T, U>::Type;

template<typename T>
using TRemoveCVRef = TRemoveCV<TRemoveReference<T>>;

template<class T> constexpr TAddConst<T>& AsConst(T& x) noexcept { return x; }
template<class T> void AsConst(const T&&) = delete;

} // namespace stp

#endif // STP_BASE_TYPE_CVREF_H_
