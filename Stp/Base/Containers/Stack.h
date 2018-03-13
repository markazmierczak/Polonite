// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_STACK_H_
#define STP_BASE_CONTAINERS_STACK_H_

#include "Base/Containers/List.h"

namespace stp {

template<typename T, class TUnderlying = List<T>>
class Stack {
 public:
  using UnderlyingType = TUnderlying;

  Stack() = default;
  explicit Stack(UnderlyingType&& u) : u_(move(u)) {}

  ALWAYS_INLINE int size() const { return u_.size(); }

  bool isEmpty() const { return u_.isEmpty(); }

  void clear() { u_.clear(); }

  template<typename U>
  bool contains(const U& item) const { return u_.contains(item); }

  const T& peek() const { return u_.last(); }
  T& peek() { return u_.last(); }

  void push(T item) { u_.add(move(item)); }
  T pop();

  const T* tryPeek() const { return isEmpty() ? nullptr : &peek(); }
  T* tryPeek() { return isEmpty() ? nullptr : &peek(); }

 private:
  UnderlyingType u_;
};

template<typename T, class TUnderlying>
inline T Stack<T, TUnderlying>::pop() {
  T value = move(u_.last());
  u_.removeLast();
  return value;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_STACK_H_
