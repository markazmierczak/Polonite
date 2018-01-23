// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_COMPARABLEFWD_H_
#define STP_BASE_TYPE_COMPARABLEFWD_H_

#include "Base/Type/Scalar.h"

namespace stp {

template<typename T, typename = void>
struct TIsTriviallyEqualityComparableTmpl :
    TBoolConstant<TIsScalar<T> && !TIsFloatingPoint<T>> {};

template<typename T>
constexpr bool TIsTriviallyEqualityComparable = TIsTriviallyEqualityComparableTmpl<T>::Value;

template<typename T, TEnableIf<TIsBoolean<T>>* = nullptr>
constexpr int Compare(T l, T r);

template<typename T, typename U, TEnableIf<TIsCharacter<T> && TIsCharacter<U>>* = nullptr>
constexpr int Compare(T lhs, U rhs);

template<typename T, typename U, TEnableIf<TIsInteger<T> && TIsInteger<U>>* = nullptr>
constexpr int Compare(T l, U r);

template<typename T, typename U, TEnableIf<TIsFloatingPoint<T> && TIsFloatingPoint<U>>* = nullptr>
constexpr int Compare(T l, U r);

template<typename T, typename U, TEnableIf<TIsPointer<T> && TIsPointer<U>>* = nullptr>
constexpr int Compare(const T& l, const U& r);

template<typename T, TEnableIf<TIsEnum<T>>* = nullptr>
constexpr int Compare(T l, T r);

namespace detail {

template<typename T, typename U>
using TEqualityComparableConcept = decltype(declval<const T&>() == declval<const U&>());
template<typename T, typename U>
using TComparableConcept = decltype(Compare(declval<const T&>(), declval<const U&>()));

} // namespace detail

template<typename T, typename U>
constexpr bool TIsEqualityComparableWith =
    TsAreSame<bool, TDetect<detail::TEqualityComparableConcept, T, U>>;

template<typename T>
constexpr bool TIsEqualityComparable = TIsEqualityComparableWith<T, T>;

template<typename T, typename U>
constexpr bool TIsComparableWith = TsAreSame<int, TDetect<detail::TComparableConcept, T, U>>;
template<typename T>
constexpr bool TIsComparable = TIsComparableWith<T, T>;

template<typename T>
inline int CompareContiguous(const T* lhs, const T* rhs, int count);

} // namespace stp

#endif // STP_BASE_TYPE_COMPARABLEFWD_H_
