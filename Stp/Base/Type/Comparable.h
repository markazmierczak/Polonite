// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_COMPARABLE_H_
#define STP_BASE_TYPE_COMPARABLE_H_

#include "Base/Containers/ArrayOps.h"
#include "Base/Type/ComparableFwd.h"

namespace stp {

template<typename T, TEnableIf<TIsBoolean<T>>*>
constexpr int Compare(T l, T r) {
  return static_cast<int>(l) - static_cast<int>(r);
}

template<typename T, typename U, TEnableIf<TIsCharacter<T> && TIsCharacter<U>>*>
constexpr int Compare(T lhs, U rhs) {
  return static_cast<int>(char_cast<char32_t>(lhs) - char_cast<char32_t>(rhs));
}

template<typename T, typename U, TEnableIf<TIsInteger<T> && TIsInteger<U>>*>
constexpr int Compare(T l, U r) {
  if (l < r)
    return -1;
  if (l > r)
    return 1;
  return 0;
}

template<typename T, typename U, TEnableIf<TIsFloatingPoint<T> && TIsFloatingPoint<U>>*>
constexpr int Compare(T l, U r) {
  if (l < r)
    return -1;
  if (l > r)
    return 1;
  if (l == r)
    return 0;
  if (IsNaN(l))
    return IsNan(r) ? 0 : -1;
  return 1;
}

template<typename T, typename U, TEnableIf<TIsPointer<T> && TIsPointer<U>>*>
constexpr int Compare(const T& l, const U& r) {
  if (l < r)
    return -1;
  if (l > r)
    return 1;
  return 0;
}

template<typename T, TEnableIf<TIsEnum<T>>*>
constexpr int Compare(T l, T r) {
  if (l < r)
    return -1;
  if (l > r)
    return 1;
  return 0;
}

namespace detail {

template<typename T, bool IsEnum = TIsEnum<T>>
struct TIsFastContiguousComparable : TFalse {};

template<> struct TIsFastContiguousComparable<char> : TTrue {};
template<> struct TIsFastContiguousComparable<unsigned char> : TTrue {};

template<typename T>
struct TIsFastContiguousComparable<T, true> : TIsFastContiguousComparable<TUnderlying<T>> {};

} // namespace detail

template<typename T>
inline int CompareContiguous(const T* lhs, const T* rhs, int count) {
  ASSERT(count >= 0);
  if constexpr (detail::TIsFastContiguousComparable<T>::Value) {
    return count ? ::memcmp(lhs, rhs, ToUnsigned(count)) : 0;
  } else {
    for (int i = 0; i < count; ++i) {
      int rv = Compare(lhs[i], rhs[i]);
      if (rv)
        return rv;
    }
    return 0;
  }
}

struct DefaultEqualityComparer {
  template<typename T, typename U>
  constexpr bool operator()(const T& x, const U& y) const {
    return x == y;
  }
};

struct DefaultComparer {
  template<typename T, typename U>
  constexpr int operator()(const T& x, const U& y) const {
    return Compare(x, y);
  }
};

} // namespace stp

#endif // STP_BASE_TYPE_COMPARABLE_H_
