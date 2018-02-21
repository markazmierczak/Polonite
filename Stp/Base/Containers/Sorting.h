// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_SORTING_H_
#define STP_BASE_CONTAINERS_SORTING_H_

#include "Base/Containers/SortingBasic.h"
#include "Base/Math/PowerOfTwo.h"

namespace stp {

template<typename T, typename TComparer = DefaultComparer>
constexpr void insertionSortSpan(MutableSpan<T> sequence, TComparer&& comparer = DefaultComparer()) {
  int right = sequence.size() - 1;
  auto* d = sequence.data();

  for (int i = 0; i < right; ++i) {
    auto tmp = move(d[i + 1]);

    int j = i;
    for (; j >= 0 && comparer(tmp, d[j]) < 0; --j)
      d[j + 1] = d[j];

    d[j + 1] = move(tmp);
  }
}

namespace detail {

template<typename T, typename TComparer>
void downHeap(T* d, int i, int n, TComparer&& comparer) {
  ASSERT(i > 0);
  ASSERT(n >= 0);

  T tmp = move(d[i - 1]);

  while (i <= (n >> 1)) {
    int child = i << 1;
    if (child < n) {
      if (comparer(d[child - 1], d[child]) < 0)
        ++child;
    }
    if (comparer(tmp, d[child - 1]) >= 0)
      break;

    d[i - 1] = move(d[child - 1]);
    i = child;
  }
  d[i - 1] = move(tmp);
}

} // namespace detail

template<typename T, typename TComparer = DefaultComparer>
void heapSortSpan(MutableSpan<T> sequence, TComparer&& comparer = DefaultComparer()) {
  auto* d = sequence.data();
  int n = sequence.size();

  // Cannot perfect forward comparer since it is used in multiple places.
  for (int i = n >> 1; i > 0; --i) {
    detail::downHeap(d, i, n, comparer);
  }
  for (int i = n - 1; i > 0; --i) {
    swap(d[0], d[i]);
    detail::downHeap(d, 1, i, comparer);
  }
}

namespace detail {

template<typename T, typename TComparer>
constexpr void swapIfGreater(T* d, int a, int b, TComparer&& comparer) {
  ASSERT(a != b);
  if (comparer(d[a], d[b]) > 0)
    swap(d[a], d[b]);
}

template<typename T, typename TComparer>
int pickPivotAndPartition(T* d, int lo, int hi, TComparer&& comparer) {
  ASSERT(lo < hi);

  int mid = getMiddleIndex(lo, hi);

  swapIfGreater(d, lo , mid, comparer);
  swapIfGreater(d, lo , hi , comparer);
  swapIfGreater(d, mid, hi , comparer);

  int left = lo;
  int right = hi - 1;

  swap(d[mid], d[right]);
  const T& pivot = d[right];

  while (left < right) {
    while (left < hi - 1) {
      ++left;
      if (comparer(pivot, d[left]) < 0)
        break;
    }
    while (right > lo) {
      --right;
      if (comparer(d[right], pivot) < 0)
        break;
    }

    if (left >= right)
      break;

    swap(d[left], d[right]);
  }
  // Move pivot to the right location.
  swap(d[left], d[hi - 1]);
  return left;
}

template<typename T, typename TComparer>
void introSort(T* d, int lo, int hi, int depth, TComparer&& comparer) {
  constexpr int PartitionSizeTreshold = 16;

  while (hi > lo) {
    int partition_size = hi - lo + 1;
    if (partition_size <= PartitionSizeTreshold) {
      if (partition_size <= 1)
        return;

      if (partition_size == 2) {
        swapIfGreater(d, lo, hi, comparer);
        return;
      }
      if (partition_size == 3) {
        swapIfGreater(d, lo, hi - 1, comparer);
        swapIfGreater(d, lo, hi, comparer);
        swapIfGreater(d, hi - 1, hi, comparer);
        return;
      }
      auto sub = MutableSpan<T>(d + lo, partition_size);
      insertionSortSpan(sub, forward<TComparer>(comparer));
      return;
    }

    if (depth == 0) {
      auto sub = MutableSpan<T>(d + lo, partition_size);
      heapSortSpan(sub, forward<TComparer>(comparer));
      return;
    }
    --depth;

    int p = pickPivotAndPartition(d, lo, hi, comparer);
    introSort(d, p + 1, hi, depth, comparer);
    hi = p - 1;
  }
}

} // namespace detail

template<typename T, typename TComparer = DefaultComparer>
inline void sortSpan(MutableSpan<T> sequence, TComparer&& comparer = DefaultComparer()) {
  if (sequence.size() <= 1)
    return;
  // use IntroSort as default algorithm.
  int depth_limit = 2 * log2Floor(sequence.size());
  detail::introSort(
      sequence.data(), 0, sequence.size() - 1,
      depth_limit,
      forward<TComparer>(comparer));
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_SORTING_H_
