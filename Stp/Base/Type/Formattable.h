// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/TextWriter.h"
#include "Base/Text/FormatFwd.h"
#include "Base/Text/StringSpan.h"
#include "Base/Type/FormattableFwd.h"
#include "Base/Type/NullableFwd.h"

namespace stp {

namespace detail {

BASE_EXPORT void FormatNull(TextWriter& out);
BASE_EXPORT void FormatBool(TextWriter& out, bool b);
BASE_EXPORT void FormatBool(TextWriter& out, bool b, const StringSpan& opts);
BASE_EXPORT void FormatChar(TextWriter& out, char32_t c, const StringSpan& opts);

BASE_EXPORT void FormatSInt32(TextWriter& out,  int32_t x);
BASE_EXPORT void FormatSInt64(TextWriter& out,  int64_t x);
BASE_EXPORT void FormatUInt32(TextWriter& out, uint32_t x);
BASE_EXPORT void FormatUInt64(TextWriter& out, uint64_t x);

BASE_EXPORT void FormatSInt32(TextWriter& out,  int32_t x, const StringSpan& opts);
BASE_EXPORT void FormatSInt64(TextWriter& out,  int64_t x, const StringSpan& opts);
BASE_EXPORT void FormatUInt32(TextWriter& out, uint32_t x, const StringSpan& opts);
BASE_EXPORT void FormatUInt64(TextWriter& out, uint64_t x, const StringSpan& opts);

template<typename T>
inline void FormatInt(TextWriter& out, T x, const StringSpan& opts) {
  if constexpr (TIsSigned<T>) {
    if constexpr (sizeof(T) <= 4)
      FormatSInt32(out, x, opts);
    else
      FormatSInt64(out, x, opts);
  } else {
    if constexpr (sizeof(T) <= 4)
      FormatUInt32(out, x, opts);
    else
      FormatUInt64(out, x, opts);
  }
}

template<typename T>
inline void FormatInt(TextWriter& out, T x) {
  if constexpr (TIsSigned<T>) {
    if constexpr (sizeof(T) <= 4)
      FormatSInt32(out, x);
    else
      FormatSInt64(out, x);
  } else {
    if constexpr (sizeof(T) <= 4)
      FormatUInt32(out, x);
    else
      FormatUInt64(out, x);
  }
}

BASE_EXPORT void FormatFloat(TextWriter& out, double x);
BASE_EXPORT void FormatFloat(TextWriter& out, double x, const StringSpan& opts);

BASE_EXPORT void FormatRawPointer(TextWriter& out, const void* ptr);

template<typename T>
using CustomContiguousFormattableConcept = decltype(
    Format(declval<TextWriter&>(), declval<const T*>(), declval<int>(), declval<const StringSpan&>()));

} // namespace detail

template<typename T, TEnableIf<TIsScalar<T>>*>
inline void Format(TextWriter& out, const T& x, const StringSpan& opts) {
  if constexpr (TIsInteger<T>) {
    detail::FormatInt(out, x, opts);
  } else if constexpr (TIsFloatingPoint<T>) {
    detail::FormatFloat(out, static_cast<double>(x), opts);
  } else if constexpr (TIsCharacter<T>) {
    detail::FormatChar(out, char_cast<char32_t>(x), opts);
  } else if constexpr (TIsEnum<T>) {
    if constexpr (TIsNamedEnum<T>) {
      out << GetEnumName(x);
    } else {
      Format(out, ToUnderlying(x), opts);
    }
  } else if constexpr (TIsBoolean<T>) {
    detail::FormatBool(out, x, opts);
  } else if constexpr (TIsPointer<T> || TIsMemberPointer<T>) {
    detail::FormatRawPointer(out, x);
  } else if constexpr (TIsNullPointer<T>) {
    detail::FormatNull(out);
  } else {
    static_assert(!TIsScalar<T>, "unknown scalar type");
  }
}

template<typename T, TEnableIf<TIsScalar<T>>*>
inline TextWriter& operator<<(TextWriter& out, const T& x) {
  if constexpr (TIsInteger<T>) {
    detail::FormatInt(out, x);
  } else if constexpr (TIsFloatingPoint<T>) {
    detail::FormatFloat(out, static_cast<double>(x));
  } else if constexpr (TIsCharacter<T>) {
    if constexpr (sizeof(T) == 1) {
      out << char_cast<char>(x);
    } else {
      out << char_cast<char32_t>(x);
    }
  } else if constexpr (TIsEnum<T>) {
    if constexpr (TIsNamedEnum<T>) {
      out << GetEnumName(x);
    } else {
      out << ToUnderlying(x);
    }
  } else if constexpr (TIsBoolean<T>) {
    detail::FormatBool(out, x);
  } else if constexpr (TIsPointer<T> || TIsMemberPointer<T>) {
    detail::FormatRawPointer(out, x);
  } else if constexpr (TIsNullPointer<T>) {
    detail::FormatNull(out);
  } else {
    static_assert(!TIsScalar<T>, "unknown scalar type");
  }
  return out;
}

namespace detail {

BASE_EXPORT void FormatContiguousGenericExt(
    TextWriter& out,
    const void* data, int size, int item_size,
    const StringSpan& opts,
    void (*item_format)(TextWriter& out, const void* item, const StringSpan& opts));

template<typename T>
inline void FormatContiguousGeneric(
    TextWriter& out, const T* data, int size, const StringSpan& opts) {
  FormatContiguousGenericExt(
      out, data, size, sizeof(T), opts,
      [](TextWriter& out, const void* item, const StringSpan& opts) {
    Format(out, *static_cast<T*>(item), opts);
  });
}

} // namespace detail

template<typename T>
inline void FormatContiguous(TextWriter& out, const T* data, int size, const StringSpan& opts) {
  if constexpr (TIsCharacter<T>) {
    out << MakeSpan(data, size);
  } else if constexpr(THasDetected<detail::CustomContiguousFormattableConcept, T>) {
    Format(out, data, size, opts);
  } else {
    detail::FormatContiguousGeneric(out, data, size, opts);
  }
}

template<typename T>
inline void FormatContiguous(TextWriter& out, const T* data, int size) {
  if constexpr (TIsCharacter<T>) {
    out << MakeSpan(data, size);
  } else if constexpr(THasDetected<detail::CustomContiguousFormattableConcept, T>) {
    Format(out, data, size);
  } else {
    detail::FormatContiguousGeneric(out, data, size, StringSpan());
  }
}

} // namespace stp
