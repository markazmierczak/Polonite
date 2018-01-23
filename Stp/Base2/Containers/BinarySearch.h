// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_BINARYSEARCH_H_
#define STP_BASE_CONTAINERS_BINARYSEARCH_H_

#include "Base/Containers/SortingBasic.h"

namespace stp {

template<typename TContainer, typename TItem, typename TComparer = DefaultComparer,
         TEnableIf<TIsContiguousContainer<TContainer>>* = nullptr>
constexpr int LowerBound(
    TContainer& sequence, const TItem& item, TComparer&& comparer = DefaultComparer()) {
  const auto* d = sequence.data();
  int lo = 0;
  int hi = sequence.size() - 1;

  while (lo < hi) {
    int i = detail::GetMiddleIndex(lo, hi);

    int c = comparer(d[i], item);
    if (c == 0) {
      while (i > 0 && comparer(d[i - 1], item) == 0)
        --i;
      lo = i;
      break;
    }

    if (c < 0)
      lo = i + 1;
    else
      hi = i - 1;
  }
  return lo;
}

template<typename TContainer, typename TItem, typename TComparer = DefaultComparer,
         TEnableIf<TIsContiguousContainer<TContainer>>* = nullptr>
constexpr int UpperBound(
    TContainer& sequence, const TItem& item, TComparer&& comparer = DefaultComparer()) {
  const auto* d = sequence.data();
  int lo = 0;
  int hi = sequence.size() - 1;

  while (lo < hi) {
    int i = detail::GetMiddleIndex(lo, hi);

    int c = comparer(d[i], item);
    if (c == 0) {
      while (i < hi && comparer(d[i + 1], item) == 0)
        ++i;
      lo = i;
      break;
    }

    if (c < 0)
      lo = i + 1;
    else
      hi = i - 1;
  }
  return lo;
}

template<typename TContainer, typename TItem, typename TComparer = DefaultComparer,
         TEnableIf<TIsContiguousContainer<TContainer>>* = nullptr>
constexpr int BinarySearch(
    TContainer& sequence, const TItem& item, TComparer&& comparer = DefaultComparer()) {
  const auto* d = sequence.data();
  int lo = 0;
  int hi = sequence.size() - 1;

  while (lo <= hi) {
    int i = detail::GetMiddleIndex(lo, hi);

    int c = comparer(d[i], item);
    if (c == 0)
      return i;

    if (c < 0)
      lo = i + 1;
    else
      hi = i - 1;
  }
  return ~lo;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_BINARYSEARCH_H_
