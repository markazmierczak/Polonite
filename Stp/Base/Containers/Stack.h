// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_STACK_H_
#define STP_BASE_CONTAINERS_STACK_H_

#include "Base/Containers/List.h"

namespace stp {

template<typename T, class TList = List<T>>
class Stack {
 public:
  using ListType = TList;

  Stack() = default;

  ALWAYS_INLINE int size() const { return list_.size(); }

  bool isEmpty() const { return list_.isEmpty(); }

  void clear() { list_.clear(); }

  template<typename U>
  bool contains(const U& item) const { return list_.contains(item); }

  const T& Peek() const { return list_.getLast(); }
  T& Peek() { return list_.getLast(); }

  void Push(T item) { list_.add(move(item)); }
  T Pop();

  const T* TryPeek() const { return isEmpty() ? nullptr : &Peek(); }
  T* TryPeek() { return isEmpty() ? nullptr : &Peek(); }

  void ensureCapacity(int request) { list_.ensureCapacity(request); }
  void willGrow(int n) { list_.willGrow(n); }

  void shrinkCapacity(int request) { list_.shrinkCapacity(request); }
  void shrinkToFit() { list_.shrinkToFit(); }

  operator Span<T>() const { return list_; }
  operator MutableSpan<T>() { return list_; }

  // Only for for-range loops.
  const T* begin() const { return list_.begin(); }
  const T* end() const { return list_.end(); }
  T* begin() { return list_.begin(); }
  T* end() { return list_.end(); }

 private:
  ListType list_;
};

template<typename T, class TList>
inline T Stack<T, TList>::Pop() {
  T value = move(list_.getLast());
  list_.RemoveLast();
  return value;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_STACK_H_
