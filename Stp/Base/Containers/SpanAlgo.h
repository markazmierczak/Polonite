// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_SPANALGO_H_
#define STP_BASE_CONTAINERS_SPANALGO_H_

#include "Base/Containers/Span.h"

namespace stp {

template<typename T, typename TPredicate>
constexpr int findIndexInSpan(Span<T> span, TPredicate&& matcher) noexcept {
  auto* d = span.data();
  for (int i = 0, s = span.size(); i < s; ++i) {
    if (matcher(d[i]))
      return i;
  }
  return -1;
}

template<typename T, typename TPredicate>
constexpr int findLastIndexInSpan(Span<T> span, TPredicate&& matcher) noexcept {
  auto* d = span.data();
  for (int i = span.size() - 1; i >= 0; --i) {
    if (matcher(d[i]))
      return i;
  }
  return -1;
}

template<typename T, typename TPredicate>
constexpr bool existsInSpan(Span<T> span, TPredicate&& matcher) noexcept {
  return findIndexInSpan(span, Forward<TPredicate>(matcher)) >= 0;
}

template<typename T, typename TItem>
inline int countInSpan(Span<T> span, const TItem& item) noexcept {
  int count = 0;
  while (true) {
    int pos = span.indexOf(item);
    if (pos < 0)
      break;
    ++count;
    span.removePrefix(pos + 1);
  }
  return count;
}

template<typename T, typename TItem, typename TPredicate>
constexpr int countMatchingInSpan(Span<T> span, const TItem& item, TPredicate&& match) noexcept {
  auto* d = span.data();
  int n = 0;
  for (int i = 0, s = span.size(); i < s; ++i) {
    if (match(d[i]))
      ++n;
  }
  return n;
}

template<typename T, typename TBefore, typename TAfter>
inline int replaceInSpan(MutableSpan<T> span, const TBefore& before, const TAfter& after) {
  int count = 0;
  while (true) {
    int pos = span.indexOf(before);
    if (pos < 0)
      break;
    ++count;
    span[pos] = after;
    span.removePrefix(pos + 1);
  }
  return count;
}

template<typename T, typename TResult>
constexpr TResult accumulateSpan(Span<T> span, TResult init) {
  auto* d = span.data();
  for (int i = 0, s = span.size(); i < s; ++i)
    init += d[i];
  return init;
}

template<typename T, typename TResult, typename TBinaryOp>
constexpr TResult accumulateSpan(Span<T> span, TResult init, TBinaryOp&& op) {
  auto* d = span.data();
  for (int i = 0, s = span.size(); i < s; ++i)
    init = op(init, d[i]);
  return init;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_SPANALGO_H_
