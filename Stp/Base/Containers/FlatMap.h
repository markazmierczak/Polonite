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

  struct KeyComparer {
    int operator()(const PairType& l, const PairType& r) const {
      return compare(l.key(), r.key());
    }
    template<typename U>
    int operator()(const U& l, const PairType& r) const {
      return compare(l, r.key());
    }
    template<typename U>
    int operator()(const PairType& l, const U& r) const {
      return compare(l.key(), r);
    }
  };

  FlatMap() = default;
  ~FlatMap() = default;

  FlatMap(FlatMap&& other) noexcept : list_(move(other.list_)) {}
  FlatMap& operator=(FlatMap&& other) noexcept { list_ = move(other.list_); return *this; }

  FlatMap(const FlatMap& other) : list_(other.list_) {}
  FlatMap& operator=(const FlatMap& other) { list_ = other.list_; return *this; }

  void willGrow(int n) { list_.willGrow(n); }
  void shrink() { list_.shrinkToFit(); }

  ALWAYS_INLINE int capacity() const { return list_.capacity(); }
  ALWAYS_INLINE int size() const { return list_.size(); }

  bool isEmpty() const { return list_.isEmpty(); }
  void clear() { list_.clear(); }

  template<typename U>
  const T& operator[](const U& key) const;
  template<typename U>
  T& operator[](const U& key);

  template<typename U>
  const T* tryGet(const U& key) const;
  template<typename U>
  T* tryGet(const U& key);

  class ConstFindResult {
   public:
    ConstFindResult(const FlatMap& that, int index) : that_(that), index_(index) {}

    explicit operator bool() const { return index_ >= 0; }

    const K& getKey() const { return that_.list_[index_].key(); }
    T& get() const { return that_.list_[index_].value(); }

   private:
    FlatMap& that_;
    int index_;
  };

  template<typename U>
  ConstFindResult find(const U& key) const {
    return ConstFindResult(*this, indexOf(key));
  }

  class FindResult {
   public:
    FindResult(FlatMap& that, int index) : that_(that), index_(index) {}

    explicit operator bool() const { return index_ >= 0; }

    const K& getKey() const { return that_.list_[index_].key(); }
    T& get() const { return that_.list_[index_].value(); }

    template<typename U>
    void add(U&& key, T value) { that_.insertAt(~index_, Forward<U>(key), move(value)); }

   private:
    FlatMap& that_;
    int index_;
  };

  template<typename U>
  FindResult find(const U& key) {
    return FindResult(*this, indexOf(key));
  }

  template<typename U>
  void set(U&& key, T value);

  template<typename U>
  T* tryAdd(U&& key, T value);

  template<typename U>
  bool tryRemove(const U& key);

  template<typename U>
  bool containsKey(const U& key) const { return indexOf(key) >= 0; }

  template<typename U>
  int indexOf(const U& key) const { return binarySearchInSpan(list_.toSpan(), key, KeyComparer()); }

  template<typename U>
  void insertAt(int at, U&& key, T value);

  void removeAt(int at) { list_.removeAt(at); }
  void removeRange(int at, int n) { list_.removeRange(at, n); }

  const K& getKeyAt(int at) const { return list_[at].key; }

  const T& getValueAt(int at) const { return list_[at].value; }
  T& getValueAt(int at) { return list_[at].value; }

  const List<PairType>& list() const { return list_; }

  static FlatMap adoptList(ListType list) {
    ASSERT(IsSorted(list));
    ASSERT(HasDuplicatesAlreadySorted(list));
    return FlatMap(OrderedUniqueTag(), move(list));
  }
  ListType releaseList() { return move(list_); }

  friend void swap(FlatMap& l, FlatMap& r) { swap(l.list_, r.list_); }
  friend bool operator==(const FlatMap& l, const FlatMap& r) { return l.list_ == r.list_; }
  friend bool operator!=(const FlatMap& l, const FlatMap& r) { return !(l == r); }
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
  const T* pvalue = tryGet(key);
  ASSERT(pvalue);
  return *pvalue;
}

template<typename K, typename T, class TList>
template<typename U>
inline T& FlatMap<K, T, TList>::operator[](const U& key) {
  T* pvalue = tryGet(key);
  ASSERT(pvalue);
  return *pvalue;
}

template<typename K, typename T, class TList>
template<typename U>
inline const T* FlatMap<K, T, TList>::tryGet(const U& key) const {
  int pos = indexOf(key);
  return pos >= 0 ? &list_[pos].value() : nullptr;
}

template<typename K, typename T, class TList>
template<typename U>
inline T* FlatMap<K, T, TList>::tryGet(const U& key) {
  int pos = indexOf(key);
  return pos >= 0 ? &list_[pos].value() : nullptr;
}

template<typename K, typename T, class TList>
template<typename U>
inline void FlatMap<K, T, TList>::set(U&& key, T new_value) {
  int pos = indexOf(key);
  if (pos >= 0) {
    auto& value = list_[pos].value();
    destroyObject(&value);
    new (&value) T(move(new_value));
  } else {
    insertAt(~pos, Forward<U>(key), move(new_value));
  }
}

template<typename K, typename T, class TList>
template<typename U>
inline T* FlatMap<K, T, TList>::tryAdd(U&& key, T value) {
  int pos = indexOf(key);
  if (pos >= 0)
    return nullptr;

  insertAt(~pos, Forward<U>(key), move(value));
  return &list_[~pos].value();
}

template<typename K, typename T, class TList>
template<typename U>
inline bool FlatMap<K, T, TList>::tryRemove(const U& key) {
  int pos = indexOf(key);
  if (pos < 0)
    return false;

  list_.removeAt(pos);
  return true;
}

template<typename K, typename T, class TList>
template<typename U>
inline void FlatMap<K, T, TList>::insertAt(int at, U&& key, T value) {
  ASSERT(at == 0 || compare(list_[at - 1].key(), key) < 0);
  ASSERT(at == size() || compare(key, list_[at].key()) < 0);
  list_.Insert(at, PairType(Forward<U>(key), move(value)));
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_FLATMAP_H_
