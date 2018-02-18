// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_SORTING_H_
#define STP_BASE_CONTAINERS_SORTING_H_

#include "Base/Containers/SortingBasic.h"
#include "Base/Math/PowerOfTwo.h"

namespace stp {

template<typename TContainer, typename TComparer = DefaultComparer,
         TEnableIf<TIsContiguousContainer<TContainer>>* = nullptr>
constexpr void InsertionSort(TContainer& sequence, TComparer&& comparer = DefaultComparer()) {
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
void DownHeap(T* d, int i, int n, TComparer&& comparer) {
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

template<typename TContainer, typename TComparer = DefaultComparer,
         TEnableIf<TIsContiguousContainer<TContainer>>* = nullptr>
void HeapSort(TContainer& sequence, TComparer&& comparer = DefaultComparer()) {
  auto* d = sequence.data();
  int n = sequence.size();

  // Cannot perfect forward comparer since it is used in multiple places.
  for (int i = n >> 1; i > 0; --i) {
    detail::DownHeap(d, i, n, comparer);
  }
  for (int i = n - 1; i > 0; --i) {
    swap(d[0], d[i]);
    detail::DownHeap(d, 1, i, comparer);
  }
}

namespace detail {

template<typename T, typename TComparer>
constexpr void SwapIfGreater(T* d, int a, int b, TComparer&& comparer) {
  ASSERT(a != b);
  if (comparer(d[a], d[b]) > 0)
    swap(d[a], d[b]);
}

template<typename T, typename TComparer>
int PickPivotAndPartition(T* d, int lo, int hi, TComparer&& comparer) {
  ASSERT(lo < hi);

  int mid = GetMiddleIndex(lo, hi);

  SwapIfGreater(d, lo , mid, comparer);
  SwapIfGreater(d, lo , hi , comparer);
  SwapIfGreater(d, mid, hi , comparer);

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
void IntroSort(T* d, int lo, int hi, int depth, TComparer&& comparer) {
  constexpr int PartitionSizeTreshold = 16;

  while (hi > lo) {
    int partition_size = hi - lo + 1;
    if (partition_size <= PartitionSizeTreshold) {
      if (partition_size <= 1)
        return;

      if (partition_size == 2) {
        SwapIfGreater(d, lo, hi, comparer);
        return;
      }
      if (partition_size == 3) {
        SwapIfGreater(d, lo, hi - 1, comparer);
        SwapIfGreater(d, lo, hi, comparer);
        SwapIfGreater(d, hi - 1, hi, comparer);
        return;
      }
      auto sub = MutableSpan<T>(d + lo, partition_size);
      InsertionSort(sub, Forward<TComparer>(comparer));
      return;
    }

    if (depth == 0) {
      auto sub = MutableSpan<T>(d + lo, partition_size);
      HeapSort(sub, Forward<TComparer>(comparer));
      return;
    }
    --depth;

    int p = PickPivotAndPartition(d, lo, hi, comparer);
    IntroSort(d, p + 1, hi, depth, comparer);
    hi = p - 1;
  }
}

} // namespace detail

template<typename TContainer, typename TComparer = DefaultComparer,
         TEnableIf<TIsContiguousContainer<TContainer>>* = nullptr>
inline void Sort(TContainer& sequence, TComparer&& comparer = DefaultComparer()) {
  if (sequence.size() <= 1)
    return;
  // use IntroSort as default algorithm.
  int depth_limit = 2 * Log2Floor(sequence.size());
  detail::IntroSort(
      sequence.data(), 0, sequence.size() - 1,
      depth_limit,
      Forward<TComparer>(comparer));
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_SORTING_H_
