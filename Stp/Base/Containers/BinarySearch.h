// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_BINARYSEARCH_H_
#define STP_BASE_CONTAINERS_BINARYSEARCH_H_

#include "Base/Containers/SortingBasic.h"

namespace stp {

template<typename T, typename TItem, typename TComparer = DefaultComparer>
constexpr int lowerBoundOfSpan(
    Span<T> sequence, const TItem& item, TComparer&& comparer = DefaultComparer()) {
  const auto* d = sequence.data();
  int lo = 0;
  int hi = sequence.size() - 1;

  while (lo < hi) {
    int i = detail::getMiddleIndex(lo, hi);

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

template<typename T, typename TItem, typename TComparer = DefaultComparer>
constexpr int upperBoundOfSpan(
    Span<T> sequence, const TItem& item, TComparer&& comparer = DefaultComparer()) {
  const auto* d = sequence.data();
  int lo = 0;
  int hi = sequence.size() - 1;

  while (lo < hi) {
    int i = detail::getMiddleIndex(lo, hi);

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

template<typename T, typename TItem, typename TComparer = DefaultComparer>
constexpr int binarySearchInSpan(
    Span<T> sequence, const TItem& item, TComparer&& comparer = DefaultComparer()) {
  const auto* d = sequence.data();
  int lo = 0;
  int hi = sequence.size() - 1;

  while (lo <= hi) {
    int i = detail::getMiddleIndex(lo, hi);

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
