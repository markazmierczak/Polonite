// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_STRINGSPAN_H_
#define STP_BASE_TEXT_STRINGSPAN_H_

#include "Base/Containers/Span.h"
#include "Base/Text/Detail/StringDetail.h"

namespace stp {

namespace detail {

template<typename T, typename = void>
struct TIsStringContainerTmpl : TFalse {};

template<typename T>
struct TIsStringContainerTmpl<T, TEnableIf<TIsContiguousContainer<T>>>
    : TIsCharacterTmpl<typename T::ItemType> {};

} // namespace detail

template<typename T, TEnableIf<TIsCharacter<T>>* = nullptr>
inline Span<T> MakeSpanFromNullTerminated(const T* cstr) {
  return MakeSpan(cstr, detail::GetLengthOfCString(cstr));
}

template<typename T>
constexpr bool TIsStringContainer = detail::TIsStringContainerTmpl<T>::Value;

template<typename TList, TEnableIf<TIsStringContainer<TList>>* = nullptr>
inline bool IsAscii(const TList& text) { return detail::IsAscii(text.data(), text.size()); }

template<typename TList, TEnableIf<TIsStringContainer<TList>>* = nullptr>
inline int IndexOfAny(const TList& list, Span<typename TList::ItemType> s) {
  return detail::IndexOfAnyCharacter(list.data(), list.size(), s.data(), s.size());
}

template<typename TList, TEnableIf<TIsStringContainer<TList>>* = nullptr>
inline int LastIndexOfAny(const TList& list, Span<typename TList::ItemType> s) {
  return detail::LastIndexOfAnyCharacter(list.data(), list.size(), s.data(), s.size());
}

template<typename TList, TEnableIf<TIsStringContainer<TList>>* = nullptr>
inline int IndexOfAnyBut(const TList& list, Span<typename TList::ItemType> s) {
  return detail::IndexOfAnyCharacterBut(list.data(), list.size(), s.data(), s.size());
}

template<typename TList, TEnableIf<TIsStringContainer<TList>>* = nullptr>
inline int LastIndexOfAnyBut(const TList& list, Span<typename TList::ItemType> s) {
  return detail::LastIndexOfAnyCharacterBut(list.data(), list.size(), s.data(), s.size());
}

} // namespace stp

#endif // STP_BASE_TEXT_STRINGSPAN_H_
