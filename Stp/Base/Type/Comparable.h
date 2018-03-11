// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_COMPARABLE_H_
#define STP_BASE_TYPE_COMPARABLE_H_

#include "Base/Containers/ArrayOps.h"
#include "Base/Type/Nullable.h"

namespace stp {

template<typename T, TEnableIf<TIsBoolean<T>>* = nullptr>
constexpr int compare(T l, T r) {
  return static_cast<int>(l) - static_cast<int>(r);
}

template<typename T, typename U, TEnableIf<TIsCharacter<T> && TIsCharacter<U>>* = nullptr>
constexpr int compare(T lhs, U rhs) {
  return static_cast<int>(charCast<char32_t>(lhs) - charCast<char32_t>(rhs));
}

template<typename T, typename U, TEnableIf<TIsInteger<T> && TIsInteger<U>>* = nullptr>
constexpr int compare(T l, U r) {
  if (l < r)
    return -1;
  if (l > r)
    return 1;
  return 0;
}

template<typename T, typename U, TEnableIf<TIsFloatingPoint<T> && TIsFloatingPoint<U>>* = nullptr>
constexpr int compare(T l, U r) {
  if (l < r)
    return -1;
  if (l > r)
    return 1;
  if (l == r)
    return 0;
  if (isNaN(l))
    return isNaN(r) ? 0 : -1;
  return 1;
}

template<typename T, typename U, TEnableIf<TIsPointer<T> && TIsPointer<U>>* = nullptr>
constexpr int compare(const T& l, const U& r) {
  if (l < r)
    return -1;
  if (l > r)
    return 1;
  return 0;
}

template<typename T, TEnableIf<TIsEnum<T>>* = nullptr>
constexpr int compare(T l, T r) {
  if (l < r)
    return -1;
  if (l > r)
    return 1;
  return 0;
}

namespace detail {

template<typename T, typename U>
using TComparableConcept = decltype(compare(declval<const T&>(), declval<const U&>()));

} // namespace detail

template<typename T, typename U>
constexpr bool TIsComparableWith = TsAreSame<int, TDetect<detail::TComparableConcept, T, U>>;
template<typename T>
constexpr bool TIsComparable = TIsComparableWith<T, T>;

struct DefaultComparer {
  template<typename T, typename U>
  constexpr int operator()(const T& x, const U& y) const {
    return compare(x, y);
  }
};

template<typename T, typename U, TEnableIf<TIsComparableWith<T, U>>* = nullptr>
constexpr int compare(const Nullable<T>& l, const Nullable<U>& r) {
  return l.operator bool() == r.operator bool()
      ? (l ? compare(*l, *r) : 0)
      : (l ? 1 : -1);
}

template<typename T, typename U, TEnableIf<TIsComparableWith<T, U>>* = nullptr>
constexpr int compare(const Nullable<T>& l, const U& r) {
  return l ? compare(*l, r) : -1;
}
template<typename T, typename U, TEnableIf<TIsComparableWith<T, U>>* = nullptr>
constexpr int compare(const T& l, const Nullable<U>& r) {
  return l ? compare(l, *r) : 1;
}

} // namespace stp

#endif // STP_BASE_TYPE_COMPARABLE_H_
