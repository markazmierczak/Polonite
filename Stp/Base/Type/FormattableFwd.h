// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_FORMATTABLEFWD_H_
#define STP_BASE_TYPE_FORMATTABLEFWD_H_

#include "Base/Containers/SpanFwd.h"
#include "Base/Type/Scalar.h"

namespace stp {

class TextWriter;

template<typename T, TEnableIf<TIsScalar<T>>* = nullptr>
inline void format(TextWriter& out, const T& x, const StringSpan& opts);

template<typename T, TEnableIf<TIsScalar<T>>* = nullptr>
inline TextWriter& operator<<(TextWriter& out, const T& x);

namespace detail {

template<typename T>
using FormattableConcept = decltype(
    format(declval<TextWriter&>(), declval<const T&>(), declval<const StringSpan&>()));

} // namespace detail

template<typename T>
constexpr bool TIsFormattable = THasDetected<detail::FormattableConcept, T>;

} // namespace stp

#endif // STP_BASE_TYPE_FORMATTABLEFWD_H_
