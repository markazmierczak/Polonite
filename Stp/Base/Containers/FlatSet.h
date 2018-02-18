// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_FLATSET_H_
#define STP_BASE_CONTAINERS_FLATSET_H_

#include "Base/Containers/BinarySearch.h"
#include "Base/Containers/List.h"
#include "Base/Containers/MapFwd.h"

namespace stp {

template<typename T, class TList>
class FlatSet {
 public:
  using ValueType = T;
  using ListType = TList;

  static_assert(TIsContiguousContainer<ListType>, "!");

  FlatSet() = default;
  ~FlatSet() = default;

  FlatSet(FlatSet&& other) noexcept : list_(move(other.list_)) {}
  FlatSet& operator=(FlatSet&& other) noexcept { list_ = move(other); return *this; }

  FlatSet(const FlatSet& other) : list_(other.list_) {}
  FlatSet& operator=(const FlatSet& other) { list_ = other; return *this; }

  void WillGrow(int n) { list_.WillGrow(n); }
  void Shrink() { list_.ShrinkToFit(); }

  ALWAYS_INLINE int capacity() const { return list_.capacity(); }
  ALWAYS_INLINE int size() const { return list_.size(); }

  bool IsEmpty() const { return list_.IsEmpty(); }
  void Clear() { list_.Clear(); }

  template<typename U>
  bool Contains(const U& value) const { return IndexOf(value) >= 0; }

  template<typename U>
  bool TryAdd(U&& value);
  template<typename U>
  bool TryRemove(const U& value);

  template<typename U>
  int IndexOf(const U& value) const { return BinarySearch(list_, value); }

  void RemoveAt(int at) { list_.RemoveAt(at); }
  void RemoveRange(int at, int n) { list_.RemoveRange(at, n); }

  const ListType& list() const { return list_; }

  static FlatSet AdoptList(ListType list) {
    ASSERT(IsSorted(list));
    ASSERT(HasDuplicatesAlreadySorted(list));
    return FlatSet(OrderedUniqueTag(), move(list));
  }
  ListType TakeList() { return move(list_); }

  friend void swap(FlatSet& l, FlatSet& r) noexcept { swap(l.list_, r.list_); }
  friend bool operator==(const FlatSet& l, const FlatSet& r) { return l.list_ == r.list_; }
  friend bool operator!=(const FlatSet& l, const FlatSet& r) { return !(l == r); }
  friend int Compare(const FlatSet& l, const FlatSet& r) { return Compare(l.list_, r.list_); }
  friend auto begin(const FlatSet& x) { return begin(x.list_); }
  friend auto end(const FlatSet& x) { return end(x.list_); }

 private:
  ListType list_;

  struct OrderedUniqueTag {};
  FlatSet(OrderedUniqueTag, ListType&& list) noexcept : list_(move(list)) {}
};

template<typename T, class TList>
struct TIsZeroConstructibleTmpl<FlatSet<T, TList>>
    : TIsZeroConstructibleTmpl<typename FlatSet<T, TList>::ListType> {};
template<typename T, class TList>
struct TIsTriviallyRelocatableTmpl<FlatSet<T, TList>>
    : TIsTriviallyRelocatableTmpl<typename FlatSet<T, TList>::ListType> {};

template<typename T, class TList>
template<typename U>
inline bool FlatSet<T, TList>::TryAdd(U&& value) {
  int pos = IndexOf(value);
  if (pos >= 0)
    return false;

  list_.Insert(~pos, T(Forward<U>(value)));
  return true;
}

template<typename T, class TList>
template<typename U>
inline bool FlatSet<T, TList>::TryRemove(const U& value) {
  int pos = IndexOf(value);
  if (pos < 0)
    return false;

  list_.RemoveAt(pos);
  return true;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_FLATSET_H_
