// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_SORTINGBASIC_H_
#define STP_BASE_CONTAINERS_SORTINGBASIC_H_

#include "Base/Containers/Span.h"
#include "Base/Type/Comparable.h"
#include "Base/Type/Variable.h"

namespace stp {

template<typename TContainer, typename TComparer = DefaultComparer,
         TEnableIf<TIsContiguousContainer<TContainer>>* = nullptr>
constexpr bool IsSorted(TContainer& sequence, TComparer&& comparer = DefaultComparer()) {
  const auto* d = sequence.data();
  for (int i = 1, s = sequence.size(); i < s; ++i) {
    if (comparer(d[i - 1], d[i]) > 0)
      return false;
  }
  return true;
}

template<typename TContainer, typename TEqualComparer = DefaultEqualityComparer,
         TEnableIf<TIsContiguousContainer<TContainer>>* = nullptr>
constexpr bool HasDuplicatesAlreadySorted(TContainer& sequence, TEqualComparer&& comparer = DefaultEqualityComparer()) {
  const auto* d = sequence.data();
  for (int i = 1, s = sequence.size(); i < s; ++i) {
    if (comparer(d[i - 1], d[i]))
      return true;
  }
  return false;
}

template<typename TContainer, TEnableIf<TIsContiguousContainer<TContainer>>* = nullptr>
constexpr void Reverse(TContainer& sequence) {
  auto* d = sequence.data();
  int left = 0;
  for (int right = sequence.size() - 1; left < right; ++left, --right)
    Swap(d[left], d[right]);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_SORTINGBASIC_H_
