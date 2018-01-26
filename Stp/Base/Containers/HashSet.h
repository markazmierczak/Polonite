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

  HashSet(HashSet&& other) : map_(Move(other.map_)) {}
  HashSet& operator=(HashSet&& other);

  void SwapWith(HashSet& other) { map_.SwapWith(other.map_); }

  int size() const { return map_.size(); }

  bool IsEmpty() const { return map_.IsEmpty(); }

  void Clear() { map_.Clear(); }

  template<typename U>
  bool TryAdd(U&& value) { return map_.TryAdd(Forward<U>(value), DummyEmpty()); }

  template<typename U>
  bool TryRemove(const U& value) { return map_.TryRemove(value); }

  template<typename U>
  bool Contains(const U& value) const { return map_.Contains(value); }

 private:
  HashMap<T, DummyEmpty> map_;
};

template<typename T>
inline HashSet<T>& HashSet<T>::operator=(HashSet&& other) {
  map_ = Move(other.map_);
  return *this;
}

template<typename T>
inline HashSet<T>& HashSet<T>::operator=(const HashSet& other) {
  map_ = other.map_;
  return *this;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_HASHSET_H_