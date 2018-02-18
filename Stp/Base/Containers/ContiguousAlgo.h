// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_CONTIGUOUSALGO_H_
#define STP_BASE_CONTAINERS_CONTIGUOUSALGO_H_

#include "Base/Containers/ArrayOps.h"
#include "Base/Containers/SpanFwd.h"

namespace stp {

template<typename TList, typename TPredicate,
         TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
constexpr int FindIndex(const TList& list, TPredicate&& matcher) {
  auto* d = list.data();
  for (int i = 0, s = list.size(); i < s; ++i) {
    if (matcher(d[i]))
      return i;
  }
  return -1;
}

template<typename TList, typename TPredicate,
         TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
constexpr int FindLastIndex(const TList& list, TPredicate&& matcher) {
  auto* d = list.data();
  for (int i = list.size() - 1; i >= 0; --i) {
    if (matcher(d[i]))
      return i;
  }
  return -1;
}

template<typename TList, typename TPredicate,
         TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
constexpr bool Exists(const TList& list, TPredicate&& matcher) {
  return FindIndex(list, Forward<TPredicate>(matcher)) >= 0;
}

template<typename TList, typename TItem, TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline int Count(const TList& list, const TItem& item) {
  return Count(list.data(), list.size(), item);
}

template<typename TList, typename TItem, typename TPredicate,
         TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline int CountMatching(const TList& list, const TItem& item, TPredicate&& match) {
  auto* d = list.data();
  int n = 0;
  for (int i = 0, s = list.size(); i < s; ++i) {
    if (match(d[i]))
      ++n;
  }
  return n;
}

template<typename TList, TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline int indexOfRange(const TList& list, Span<typename TList::ItemType> range) {
  return indexOfRange(list.data(), list.size(), range.data(), range.size());
}

template<typename TList, TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline int lastIndexOfRange(const TList& list, Span<typename TList::ItemType> range) {
  return lastIndexOfRange(list.data(), list.size(), range.data(), range.size());
}

template<typename TList, TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline bool containsRange(const TList& list, Span<typename TList::ItemType> range) {
  return indexOfRange(list, range) >= 0;
}

template<typename TList, TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline bool StartsWith(const TList& list, Span<typename TList::ItemType> prefix) {
  return list.size() >= prefix.size() && list.getSlice(0, prefix.size()) == prefix;
}

template<typename TList, TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline bool EndsWith(const TList& list, Span<typename TList::ItemType> suffix) {
  return list.size() >= suffix.size() && list.getSlice(list.size() - suffix.size()) == suffix;
}

template<typename TList, typename TItem, TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline bool TryRemoveOne(TList& list, const TItem& item) {
  int index = list.indexOf(item);
  if (index >= 0) {
    list.RemoveAt(index);
    return true;
  }
  return false;
}

template<typename TList, typename TPredicate,
         TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline bool TryRemoveOneMatching(TList& list, TPredicate&& match) {
  int index = FindIndex(list, Forward<TPredicate>(match));
  if (index >= 0) {
    list.RemoveAt(index);
    return true;
  }
  return false;
}

template<typename TList, typename TPredicate,
         TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
int RemoveAllMatching(TList& list, TPredicate&& match) {
  int removed_count = 0;
  auto* d = list.data();
  int chunk_end = list.size();
  while (chunk_end > 0) {
    // Skip valid items.
    int chunk_start = chunk_end;
    for (; chunk_start > 0; --chunk_start) {
      if (match(d[chunk_start - 1]))
        break;
    }
    chunk_end = chunk_start;
    for (; chunk_start > 0; --chunk_start) {
      if (!match(d[chunk_start - 1]))
        break;
    }
    if (chunk_start != chunk_end) {
      int chunk_length = chunk_end - chunk_start;
      list.RemoveRange(chunk_start, chunk_length);
      removed_count += chunk_length;
      chunk_end = chunk_start;
    }
  }
  return removed_count;
}

template<typename TList, typename TItem, typename TPredicate,
         TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline int RemoveAll(TList& list, const TItem& item) {
  return RemoveAllMatching(list, [&item](decltype(*list.data()) x) {
    return x == item;
  });
}

template<typename TList, typename TItem, TEnableIf<TIsContiguousContainer<TRemoveReference<TList>>>* = nullptr>
inline void Fill(TList&& list, const TItem& item) {
  Fill(list.data(), list.size(), item);
}

template<typename TList,
         typename TBefore, typename TAfter,
         TEnableIf<TIsContiguousContainer<TRemoveReference<TList>>>* = nullptr>
inline int Replace(TList&& list, const TBefore& before, const TAfter& after) {
  return Replace(list.data(), list.size(), before, after);
}

template<typename T, typename TList, TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline T Accumulate(const TList& list, T init) {
  auto* d = list.data();
  for (int i = 0, s = list.size(); i < s; ++i)
    init += d[i];
  return init;
}

template<typename T, typename TBinaryOp,
         typename TList, TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline T Accumulate(const TList& list, T init, TBinaryOp&& op) {
  auto* d = list.data();
  for (int i = 0, s = list.size(); i < s; ++i)
    init = op(init, d[i]);
  return init;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_CONTIGUOUSALGO_H_
