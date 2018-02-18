// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_STACK_H_
#define STP_BASE_CONTAINERS_STACK_H_

#include "Base/Containers/List.h"
#include "Base/Containers/InlineListFwd.h"

namespace stp {

template<typename T, class TList = List<T>>
class Stack {
 public:
  using ListType = TList;

  Stack() = default;

  ALWAYS_INLINE int size() const { return list_.size(); }

  bool IsEmpty() const { return list_.IsEmpty(); }

  void Clear() { list_.Clear(); }

  template<typename U>
  bool Contains(const U& item) const { return list_.Contains(item); }

  const T& Peek() const { return list_.GetLast(); }
  T& Peek() { return list_.GetLast(); }

  void Push(T item) { list_.Add(move(item)); }
  T Pop();

  const T* TryPeek() const { return IsEmpty() ? nullptr : &Peek(); }
  T* TryPeek() { return IsEmpty() ? nullptr : &Peek(); }

  void EnsureCapacity(int request) { list_.EnsureCapacity(request); }
  void WillGrow(int n) { list_.WillGrow(n); }

  void ShrinkCapacity(int request) { list_.ShrinkCapacity(request); }
  void ShrinkToFit() { list_.ShrinkToFit(); }

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
  T value = move(list_.GetLast());
  list_.RemoveLast();
  return value;
}

template<typename T, int N>
using InlineStack = Stack<T, InlineList<T, N>>;

} // namespace stp

#endif // STP_BASE_CONTAINERS_STACK_H_
