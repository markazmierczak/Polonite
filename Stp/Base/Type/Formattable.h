// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_FORMATTABLE_H_
#define STP_BASE_TYPE_FORMATTABLE_H_

#include "Base/Io/TextWriter.h"

namespace stp {

namespace detail {

BASE_EXPORT void formatNull(TextWriter& out);
BASE_EXPORT void formatBool(TextWriter& out, bool b);
BASE_EXPORT void formatBool(TextWriter& out, bool b, const StringSpan& opts);
BASE_EXPORT void formatChar(TextWriter& out, char32_t c, const StringSpan& opts);

BASE_EXPORT void formatSint32(TextWriter& out,  int32_t x);
BASE_EXPORT void formatSint64(TextWriter& out,  int64_t x);
BASE_EXPORT void formatUint32(TextWriter& out, uint32_t x);
BASE_EXPORT void formatUint64(TextWriter& out, uint64_t x);

BASE_EXPORT void formatSint32(TextWriter& out,  int32_t x, const StringSpan& opts);
BASE_EXPORT void formatSint64(TextWriter& out,  int64_t x, const StringSpan& opts);
BASE_EXPORT void formatUint32(TextWriter& out, uint32_t x, const StringSpan& opts);
BASE_EXPORT void formatUint64(TextWriter& out, uint64_t x, const StringSpan& opts);

template<typename T>
inline void formatInt(TextWriter& out, T x, const StringSpan& opts) {
  if constexpr (TIsSigned<T>) {
    if constexpr (sizeof(T) <= 4)
      formatSint32(out, x, opts);
    else
      formatSint64(out, x, opts);
  } else {
    if constexpr (sizeof(T) <= 4)
      formatUint32(out, x, opts);
    else
      formatUint64(out, x, opts);
  }
}

template<typename T>
inline void formatInt(TextWriter& out, T x) {
  if constexpr (TIsSigned<T>) {
    if constexpr (sizeof(T) <= 4)
      formatSint32(out, x);
    else
      formatSint64(out, x);
  } else {
    if constexpr (sizeof(T) <= 4)
      formatUint32(out, x);
    else
      formatUint64(out, x);
  }
}

BASE_EXPORT void formatFloat(TextWriter& out, double x);
BASE_EXPORT void formatFloat(TextWriter& out, double x, const StringSpan& opts);

BASE_EXPORT void formatRawPointer(TextWriter& out, const void* ptr);

template<typename T>
using CustomContiguousFormattableConcept = decltype(
    format(declval<TextWriter&>(), declval<const T*>(), declval<int>(), declval<const StringSpan&>()));

} // namespace detail

template<typename T, TEnableIf<TIsScalar<T>>*>
inline void format(TextWriter& out, const T& x, const StringSpan& opts) {
  if constexpr (TIsInteger<T>) {
    detail::formatInt(out, x, opts);
  } else if constexpr (TIsFloatingPoint<T>) {
    detail::formatFloat(out, static_cast<double>(x), opts);
  } else if constexpr (TIsCharacter<T>) {
    detail::formatChar(out, charCast<char32_t>(x), opts);
  } else if constexpr (TIsEnum<T>) {
    if constexpr (TIsNamedEnum<T>) {
      out << getEnumName(x);
    } else {
      format(out, toUnderlying(x), opts);
    }
  } else if constexpr (TIsBoolean<T>) {
    detail::formatBool(out, x, opts);
  } else if constexpr (TIsPointer<T> || TIsMemberPointer<T>) {
    detail::formatRawPointer(out, x);
  } else if constexpr (TIsNullPointer<T>) {
    detail::formatNull(out);
  } else {
    static_assert(!TIsScalar<T>, "unknown scalar type");
  }
}

template<typename T, TEnableIf<TIsScalar<T>>*>
inline TextWriter& operator<<(TextWriter& out, const T& x) {
  if constexpr (TIsInteger<T>) {
    detail::formatInt(out, x);
  } else if constexpr (TIsFloatingPoint<T>) {
    detail::formatFloat(out, static_cast<double>(x));
  } else if constexpr (TIsCharacter<T>) {
    if constexpr (sizeof(T) == 1) {
      out << charCast<char>(x);
    } else {
      out << charCast<char32_t>(x);
    }
  } else if constexpr (TIsEnum<T>) {
    if constexpr (TIsNamedEnum<T>) {
      out << getEnumName(x);
    } else {
      out << toUnderlying(x);
    }
  } else if constexpr (TIsBoolean<T>) {
    detail::formatBool(out, x);
  } else if constexpr (TIsPointer<T> || TIsMemberPointer<T>) {
    detail::formatRawPointer(out, x);
  } else if constexpr (TIsNullPointer<T>) {
    detail::formatNull(out);
  } else {
    static_assert(!TIsScalar<T>, "unknown scalar type");
  }
  return out;
}

inline void format(TextWriter& out, StringSpan text, const StringSpan& opts) {
  out << text;
}

namespace detail {

template<typename T>
using FormattableConcept = decltype(declval<TextWriter&>() << declval<const T&>());

template<typename T>
using FormattableExtendedConcept = decltype(
    format(declval<TextWriter&>(), declval<const T&>(), declval<const StringSpan&>()));


} // namespace detail

template<typename T>
constexpr bool TIsFormattable = THasDetected<detail::FormattableConcept, T>;
template<typename T>
constexpr bool TIsFormattableExtended = THasDetected<detail::FormattableExtendedConcept, T>;

} // namespace stp

#endif // STP_BASE_TYPE_FORMATTABLE_H_
