// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_FLATMAP_H_
#define STP_BASE_CONTAINERS_FLATMAP_H_

#include "Base/Containers/BinarySearch.h"
#include "Base/Containers/KeyValuePair.h"
#include "Base/Containers/List.h"
#include "Base/Containers/MapFwd.h"
#include "Base/Containers/SortingBasic.h"
#include "Base/Type/Comparable.h"

namespace stp {

template<typename K, typename T, class TList>
class FlatMap {
 public:
  using KeyType = K;
  using ValueType = T;
  using PairType = KeyValuePair<K, T>;
  using ListType = TList;

  static_assert(TIsContiguousContainer<ListType>, "!");

  struct KeyComparer {
    int operator()(const PairType& l, const PairType& r) const {
      return Compare(l.key(), r.key());
    }
    template<typename U>
    int operator()(const U& l, const PairType& r) const {
      return Compare(l, r.key());
    }
    template<typename U>
    int operator()(const PairType& l, const U& r) const {
      return Compare(l.key(), r);
    }
  };

  FlatMap() = default;
  ~FlatMap() = default;

  FlatMap(FlatMap&& other) noexcept : list_(move(other.list_)) {}
  FlatMap& operator=(FlatMap&& other) noexcept { list_ = move(other.list_); return *this; }

  FlatMap(const FlatMap& other) : list_(other.list_) {}
  FlatMap& operator=(const FlatMap& other) { list_ = other.list_; return *this; }

  void WillGrow(int n) { list_.WillGrow(n); }
  void Shrink() { list_.ShrinkToFit(); }

  ALWAYS_INLINE int capacity() const { return list_.capacity(); }
  ALWAYS_INLINE int size() const { return list_.size(); }

  bool IsEmpty() const { return list_.IsEmpty(); }
  void Clear() { list_.Clear(); }

  template<typename U>
  const T& operator[](const U& key) const;
  template<typename U>
  T& operator[](const U& key);

  template<typename U>
  const T* TryGet(const U& key) const;
  template<typename U>
  T* TryGet(const U& key);

  class ConstFindResult {
   public:
    ConstFindResult(const FlatMap& that, int index) : that_(that), index_(index) {}

    explicit operator bool() const { return index_ >= 0; }

    const K& GetKey() const { return that_.list_[index_].key(); }
    T& Get() const { return that_.list_[index_].value(); }

   private:
    FlatMap& that_;
    int index_;
  };

  template<typename U>
  ConstFindResult Find(const U& key) const {
    return ConstFindResult(*this, IndexOf(key));
  }

  class FindResult {
   public:
    FindResult(FlatMap& that, int index) : that_(that), index_(index) {}

    explicit operator bool() const { return index_ >= 0; }

    const K& GetKey() const { return that_.list_[index_].key(); }
    T& Get() const { return that_.list_[index_].value(); }

    template<typename U>
    void Add(U&& key, T value) { that_.InsertAt(~index_, Forward<U>(key), move(value)); }

   private:
    FlatMap& that_;
    int index_;
  };

  template<typename U>
  FindResult Find(const U& key) {
    return FindResult(*this, IndexOf(key));
  }

  template<typename U>
  void Set(U&& key, T value);

  template<typename U>
  T* TryAdd(U&& key, T value);

  template<typename U>
  bool TryRemove(const U& key);

  template<typename U>
  bool ContainsKey(const U& key) const { return IndexOf(key) >= 0; }

  template<typename U>
  int IndexOf(const U& key) const { return BinarySearch(list_, key, KeyComparer()); }

  template<typename U>
  void InsertAt(int at, U&& key, T value);

  void RemoveAt(int at) { list_.RemoveAt(at); }
  void RemoveRange(int at, int n) { list_.RemoveRange(at, n); }

  const K& GetKeyAt(int at) const { return list_[at].key; }

  const T& GetValueAt(int at) const { return list_[at].value; }
  T& GetValueAt(int at) { return list_[at].value; }

  const List<PairType>& list() const { return list_; }

  static FlatMap AdoptList(ListType list) {
    ASSERT(IsSorted(list));
    ASSERT(HasDuplicatesAlreadySorted(list));
    return FlatMap(OrderedUniqueTag(), move(list));
  }
  ListType TakeList() { return move(list_); }

  friend void Swap(FlatMap& l, FlatMap& r) { Swap(l.list_, r.list_); }
  friend bool operator==(const FlatMap& l, const FlatMap& r) { return l.list_ == r.list_; }
  friend bool operator!=(const FlatMap& l, const FlatMap& r) { return !(l == r); }
  friend HashCode Hash(const FlatMap& x) { return Hash(x.list_); }
  friend int Compare(const FlatMap& l, FlatMap r) { return Compare(l.list_, r.list_); }
  friend const PairType* begin(const FlatMap& x) { return begin(x.list_); }
  friend const PairType* end(const FlatMap& x) { return end(x.list_); }
  friend PairType* begin(FlatMap& x) { return begin(x.list_); }
  friend PairType* end(FlatMap& x) { return end(x.list_); }

 private:
  ListType list_;

  struct OrderedUniqueTag {};
  FlatMap(OrderedUniqueTag, ListType&& list) noexcept : list_(move(list)) {}
};

template<typename K, typename T, class TList>
struct TIsZeroConstructibleTmpl<FlatMap<K, T, TList>>
    : TIsZeroConstructibleTmpl<typename FlatMap<K, T, TList>::ListType> {};

template<typename K, typename T, class TList>
struct TIsTriviallyRelocatableTmpl<FlatMap<K, T, TList>>
    : TIsTriviallyRelocatableTmpl<typename FlatMap<K, T, TList>::ListType> {};

template<typename K, typename T, class TList>
template<typename U>
inline const T& FlatMap<K, T, TList>::operator[](const U& key) const {
  const T* pvalue = TryGet(key);
  ASSERT(pvalue);
  return *pvalue;
}

template<typename K, typename T, class TList>
template<typename U>
inline T& FlatMap<K, T, TList>::operator[](const U& key) {
  T* pvalue = TryGet(key);
  ASSERT(pvalue);
  return *pvalue;
}

template<typename K, typename T, class TList>
template<typename U>
inline const T* FlatMap<K, T, TList>::TryGet(const U& key) const {
  int pos = IndexOf(key);
  return pos >= 0 ? &list_[pos].value() : nullptr;
}

template<typename K, typename T, class TList>
template<typename U>
inline T* FlatMap<K, T, TList>::TryGet(const U& key) {
  int pos = IndexOf(key);
  return pos >= 0 ? &list_[pos].value() : nullptr;
}

template<typename K, typename T, class TList>
template<typename U>
inline void FlatMap<K, T, TList>::Set(U&& key, T value) {
  int pos = IndexOf(key);
  if (pos >= 0) {
    auto& value = list_[pos].value();
    DestroyAt(&value);
    new (&value) T(move(value));
  } else {
    InsertAt(~pos, Forward<U>(key), move(value));
  }
}

template<typename K, typename T, class TList>
template<typename U>
inline T* FlatMap<K, T, TList>::TryAdd(U&& key, T value) {
  int pos = IndexOf(key);
  if (pos >= 0)
    return nullptr;

  InsertAt(~pos, Forward<U>(key), move(value));
  return &list_[~pos].value();
}

template<typename K, typename T, class TList>
template<typename U>
inline bool FlatMap<K, T, TList>::TryRemove(const U& key) {
  int pos = IndexOf(key);
  if (pos < 0)
    return false;

  list_.RemoveAt(pos);
  return true;
}

template<typename K, typename T, class TList>
template<typename U>
inline void FlatMap<K, T, TList>::InsertAt(int at, U&& key, T value) {
  ASSERT(at == 0 || Compare(list_[at - 1].key(), key) < 0);
  ASSERT(at == size() || Compare(key, list_[at].key()) < 0);
  list_.Insert(at, PairType(Forward<U>(key), move(value)));
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_FLATMAP_H_
