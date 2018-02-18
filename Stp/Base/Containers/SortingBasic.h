// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_SORTINGBASIC_H_
#define STP_BASE_CONTAINERS_SORTINGBASIC_H_

#include "Base/Containers/Span.h"
#include "Base/Type/Comparable.h"
#include "Base/Type/Variable.h"

namespace stp {

namespace detail {

constexpr int getMiddleIndex(int lower, int upper) {
  ASSERT(lower <= upper);
  return lower + ((upper - lower) >> 1);
}

} // namespace detail

template<typename T, typename TComparer = DefaultComparer>
constexpr bool isSortedSpan(Span<T> sequence, TComparer&& comparer = DefaultComparer()) {
  const auto* d = sequence.data();
  for (int i = 1, s = sequence.size(); i < s; ++i) {
    if (comparer(d[i - 1], d[i]) > 0)
      return false;
  }
  return true;
}

template<typename T, typename TEqualComparer = DefaultEqualityComparer>
constexpr bool hasDuplicatesAlreadySortedInSpan(Span<T> sequence, TEqualComparer&& comparer = DefaultEqualityComparer()) {
  const auto* d = sequence.data();
  for (int i = 1, s = sequence.size(); i < s; ++i) {
    if (comparer(d[i - 1], d[i]))
      return true;
  }
  return false;
}

template<typename T>
constexpr void reverseSpan(MutableSpan<T> sequence) {
  auto* d = sequence.data();
  int s = sequence.size();
  for (int i = 0; i < (s >> 1); ++i)
    swap(d[i], d[s - 1 - i]);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_SORTINGBASIC_H_
