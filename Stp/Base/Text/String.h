// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_STRING_H_
#define STP_BASE_TEXT_STRING_H_

#include "Base/Containers/List.h"
#include "Base/Text/StringSpan.h"

namespace stp {

template<typename TList, TEnableIf<TIsStringContainer<TList>>* = nullptr>
int Replace(TList& list, Span<typename TList::ItemType> from, Span<typename TList::ItemType> to) {
  ASSERT(!from.IsEmpty());
  ASSERT(!list.IsSourceOf(from) && !list.IsSourceOf(to));

  // TODO Replace this implementation with Search and Boyer-Moore searcher.
  int pos = 0;
  int end = list.size() - to.size();

  int count = 0;
  if (from.size() == to.size()) {
    if (from == to)
      return 0;
    do {
      int next_pos = pos + IndexOfRange(list.GetSlice(pos), from);
      if (next_pos < pos)
        break;
      CopyNonOverlapping(list.data() + pos, to.data(), to.size());
      pos += to.size();
    } while (pos <= end);
  } else if (ContainsRange(list, from)) {
    TRemoveCV<TList> orig = TMove(list);
    list.EnsureCapacity(orig.size());

    do {
      int skip = IndexOfRange(orig.GetSlice(pos), from);
      list.Append(orig.GetSlice(pos, skip));
      list.Append(to);
      pos += skip + from.size();
    } while (pos <= end);
    list.Append(orig.GetSlice(pos));
  }
  return count;
}

} // namespace stp

#endif // STP_BASE_TEXT_STRING_H_
