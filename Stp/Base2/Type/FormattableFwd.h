// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_FORMATTABLEFWD_H_
#define STP_BASE_TYPE_FORMATTABLEFWD_H_

#include "Base/Containers/SpanFwd.h"
#include "Base/Type/Scalar.h"

namespace stp {

class TextWriter;

template<typename T, TEnableIf<TIsScalar<T>>* = nullptr>
inline void Format(TextWriter& out, const T& x, const StringSpan& opts);

template<typename T, TEnableIf<TIsScalar<T>>* = nullptr>
inline TextWriter& operator<<(TextWriter& out, const T& x);

namespace detail {

template<typename T>
using FormattableConcept = decltype(
    Format(declval<TextWriter&>(), declval<const T&>(), declval<const StringSpan&>()));

} // namespace detail

template<typename T>
constexpr bool TIsFormattable = THasDetected<detail::FormattableConcept, T>;

template<typename T>
inline void FormatContiguous(TextWriter& out, const T* data, int size, const StringSpan& opts);
template<typename T>
inline void FormatContiguous(TextWriter& out, const T* data, int size);

template<typename T, int N, TEnableIf<TIsFormattable<T>>* = nullptr>
inline void Format(TextWriter& out, const T (&array)[N], const StringSpan& opts) {
  FormatContiguous(out, static_cast<const T*>(array), N, opts);
}
template<typename T, int N, TEnableIf<TIsFormattable<T>>* = nullptr>
inline TextWriter& operator<<(TextWriter& out, const T (&array)[N]) {
  FormatContiguous(out, static_cast<const T*>(array), N); return out;
}

BASE_EXPORT void FormatBuffer(TextWriter& out, const void* data, int size, const StringSpan& opts);
BASE_EXPORT void FormatBuffer(TextWriter& out, const void* data, int size);
BASE_EXPORT void FormatBuffer(MutableStringSpan out, const void* data, int size, bool uppercase);

} // namespace stp

#endif // STP_BASE_TYPE_FORMATTABLEFWD_H_
