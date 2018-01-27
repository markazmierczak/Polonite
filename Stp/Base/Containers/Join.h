// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_JOIN_H_
#define STP_BASE_CONTAINERS_JOIN_H_

#include "Base/Containers/List.h"

namespace stp {

template<typename TResult, typename TArray>
inline TResult Concat(const TArray& inputs) {
  static_assert(TIsContiguousContainer<TArray>, "!");
  static_assert(TIsContiguousContainer<TResult>, "!");

  TResult result;

  int result_length = 0;
  for (int i = 0; i < inputs.size(); ++i)
    result_length += inputs[i].size();

  result.EnsureCapacity(result_length);

  for (int i = 0; i < inputs.size(); ++i)
    result.Append(inputs[i]);
  return result;
}

template<typename TResult, typename... TArgs>
inline TResult ConcatMany(const TArgs&... args) {
  using ItemType = typename TResult::ItemType;
  InitializerList<Span<ItemType>> ilist = { args... };
  return Concat<TResult>(MakeSpan(ilist));
}

template<typename TResult, typename TArray>
inline TResult Join(typename TResult::ItemType separator, const TArray& inputs) {
  static_assert(TIsContiguousContainer<TArray>, "!");
  static_assert(TIsContiguousContainer<TResult>, "!");

  TResult result;
  // Must check for empty due to later decrement when computing output length.
  if (!inputs.IsEmpty()) {
    int result_length = 0;
    for (int i = 0; i < inputs.size(); ++i)
      result_length += inputs[i].size();
    result_length += inputs.size() - 1;

    result.EnsureCapacity(result_length);

    for (int i = 0; i < inputs.size(); ++i) {
      if (i != 0) {
        result.Add(separator);
      }
      result.Append(inputs[i]);
    }
  }
  return result;
}

template<typename TResult, typename... TArgs>
inline TResult JoinMany(typename TResult::ItemType separator, const TArgs&... args) {
  using ItemType = typename TResult::ItemType;
  InitializerList<Span<ItemType>> ilist = { args... };
  return Join<TResult>(Move(separator), MakeSpan(ilist));
}

template<typename TResult, typename TArray>
inline TResult Join(Span<typename TResult::ItemType> separator, const TArray& inputs) {
  static_assert(TIsContiguousContainer<TArray>, "!");
  static_assert(TIsContiguousContainer<TResult>, "!");

  TResult result;
  // Must check for empty due to later decrement when computing output length.
  if (!inputs.IsEmpty()) {
    int result_length = 0;
    for (int i = 0; i < inputs.size(); ++i)
      result_length += inputs[i].size();
    result_length += (inputs.size() - 1) * separator.size();

    result.EnsureCapacity(result_length);

    for (int i = 0; i < inputs.size(); ++i) {
      if (i != 0) {
        result.Append(separator);
      }
      result.Append(inputs[i]);
    }
  }
  return result;
}

template<typename TResult, typename... TArgs>
inline TResult JoinMany(Span<typename TResult::ItemType> separator, const TArgs&... args) {
  using ItemType = typename TResult::ItemType;
  InitializerList<Span<ItemType>> ilist = { args... };
  return Join<TResult>(separator, MakeSpan(ilist));
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_JOIN_H_