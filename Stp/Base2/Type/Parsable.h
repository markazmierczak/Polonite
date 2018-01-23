// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_PARSABLE_H_
#define STP_BASE_TYPE_PARSABLE_H_

#include "Base/Error/BasicExceptions.h"

namespace stp {

enum class ParseIntegerErrorCode;

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr ParseIntegerErrorCode TryParse(StringSpan input, T& output);

template<typename TResult, typename = void>
struct ParseTmpl {
  constexpr void operator()(StringSpan text, TResult& rv) {
    if (!TryParse(text, rv))
      throw FormatException();
  }
};

template<typename T>
struct ParseTmpl<T, TEnableIf<TIsInteger<T>>>;

template<typename TResult>
constexpr void Parse(StringSpan text, TResult& result) {
  ParseTmpl<TResult>()(text, result);
}

template<typename TResult>
constexpr TResult ParseTo(StringSpan text) {
  auto result = TResult();
  Parse(text, result);
  return result;
}

} // namespace stp

#endif // STP_BASE_TYPE_PARSABLE_H_
