// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_PARSABLE_H_
#define STP_BASE_TYPE_PARSABLE_H_

namespace stp {

enum class ParseIntegerErrorCode;

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr ParseIntegerErrorCode tryParse(StringSpan input, T& output);

} // namespace stp

#endif // STP_BASE_TYPE_PARSABLE_H_
