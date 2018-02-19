// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_LISTALGO_H_
#define STP_BASE_CONTAINERS_LISTALGO_H_

#include "Base/Containers/List.h"

namespace stp {

template<typename TList, typename TItem, TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline bool tryRemoveOne(TList& list, const TItem& item) {
  int index = list.indexOf(item);
  if (index >= 0) {
    list.removeAt(index);
    return true;
  }
  return false;
}

template<typename TList, typename TPredicate,
         TEnableIf<TIsContiguousContainer<TList>>* = nullptr>
inline bool tryRemoveOneMatching(TList& list, TPredicate&& match) {
  int index = FindIndex(list, Forward<TPredicate>(match));
  if (index >= 0) {
    list.removeAt(index);
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
      list.removeRange(chunk_start, chunk_length);
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

} // namespace stp

#endif // STP_BASE_CONTAINERS_LISTALGO_H_
