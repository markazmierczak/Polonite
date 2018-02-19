// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_HASHSET_H_
#define STP_BASE_CONTAINERS_HASHSET_H_

#include "Base/Containers/HashMap.h"

namespace stp {

template<typename T>
class HashSet {
 private:
  struct DummyEmpty {};

 public:
  HashSet() = default;
  ~HashSet() = default;

  HashSet(const HashSet& other) : map_(other.map_) {}
  HashSet& operator=(const HashSet& other);

  HashSet(HashSet&& other) : map_(move(other.map_)) {}
  HashSet& operator=(HashSet&& other);

  int size() const { return map_.size(); }

  bool isEmpty() const { return map_.isEmpty(); }

  void clear() { map_.clear(); }

  template<typename U>
  bool tryAdd(U&& value) { return map_.tryAdd(Forward<U>(value), DummyEmpty()); }

  template<typename U>
  bool tryRemove(const U& value) { return map_.tryRemove(value); }

  template<typename U>
  bool contains(const U& value) const { return map_.contains(value); }

  friend void swap(HashSet& lhs, HashSet& rhs) { swap(lhs.map_, rhs.map_); }

 private:
  HashMap<T, DummyEmpty> map_;
};

template<typename T>
inline HashSet<T>& HashSet<T>::operator=(HashSet&& other) {
  map_ = move(other.map_);
  return *this;
}

template<typename T>
inline HashSet<T>& HashSet<T>::operator=(const HashSet& other) {
  map_ = other.map_;
  return *this;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_HASHSET_H_
