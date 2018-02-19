// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_JOIN_H_
#define STP_BASE_CONTAINERS_JOIN_H_

#include "Base/Containers/List.h"

namespace stp {

template<typename TResult, typename TArray>
inline TResult Concat(const TArray& inputs) {
  TResult result;

  int result_length = 0;
  for (int i = 0; i < inputs.size(); ++i)
    result_length += inputs[i].size();

  result.ensureCapacity(result_length);

  for (int i = 0; i < inputs.size(); ++i)
    result.append(inputs[i]);
  return result;
}

template<typename TResult, typename... TArgs>
inline TResult ConcatMany(const TArgs&... args) {
  using ItemType = typename TResult::ItemType;
  InitializerList<Span<ItemType>> ilist = { args... };
  return Concat<TResult>(makeSpan(ilist));
}

template<typename TResult, typename TArray>
inline TResult Join(typename TResult::ItemType separator, const TArray& inputs) {
  TResult result;
  // Must check for empty due to later decrement when computing output length.
  if (!inputs.isEmpty()) {
    int result_length = 0;
    for (int i = 0; i < inputs.size(); ++i)
      result_length += inputs[i].size();
    result_length += inputs.size() - 1;

    result.ensureCapacity(result_length);

    for (int i = 0; i < inputs.size(); ++i) {
      if (i != 0) {
        result.add(separator);
      }
      result.append(inputs[i]);
    }
  }
  return result;
}

template<typename TResult, typename... TArgs>
inline TResult JoinMany(typename TResult::ItemType separator, const TArgs&... args) {
  using ItemType = typename TResult::ItemType;
  InitializerList<Span<ItemType>> ilist = { args... };
  return Join<TResult>(move(separator), makeSpan(ilist));
}

template<typename TResult, typename TArray>
inline TResult Join(Span<typename TResult::ItemType> separator, const TArray& inputs) {
  TResult result;
  // Must check for empty due to later decrement when computing output length.
  if (!inputs.isEmpty()) {
    int result_length = 0;
    for (int i = 0; i < inputs.size(); ++i)
      result_length += inputs[i].size();
    result_length += (inputs.size() - 1) * separator.size();

    result.ensureCapacity(result_length);

    for (int i = 0; i < inputs.size(); ++i) {
      if (i != 0) {
        result.append(separator);
      }
      result.append(inputs[i]);
    }
  }
  return result;
}

template<typename TResult, typename... TArgs>
inline TResult JoinMany(Span<typename TResult::ItemType> separator, const TArgs&... args) {
  using ItemType = typename TResult::ItemType;
  InitializerList<Span<ItemType>> ilist = { args... };
  return Join<TResult>(separator, makeSpan(ilist));
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_JOIN_H_
