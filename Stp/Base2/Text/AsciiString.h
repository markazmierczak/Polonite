// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_ASCIISTRING_H_
#define STP_BASE_TEXT_ASCIISTRING_H_

#include "Base/Text/AsciiChar.h"
#include "Base/Text/StringSpan.h"

namespace stp {

namespace detail {

BASE_EXPORT int CompareIgnoreCaseAscii(const char* lhs, const char* rhs, int size);

} // namespace detail

inline bool EqualsIgnoreCaseAscii(StringSpan lhs, StringSpan rhs) {
  return lhs.size() == rhs.size() &&
      detail::CompareIgnoreCaseAscii(lhs.data(), rhs.data(), lhs.size()) == 0;
}

inline int CompareIgnoreCaseAscii(StringSpan lhs, StringSpan rhs) {
  int min_size = Min(lhs.size(), rhs.size());
  int rv = detail::CompareIgnoreCaseAscii(lhs.data(), rhs.data(), min_size);
  return rv != 0 ? rv : (lhs.size() - rhs.size());
}

struct IgnoreCaseAsciiComparer {
  int operator()(StringSpan lhs, StringSpan rhs) const { return CompareIgnoreCaseAscii(lhs, rhs); }
};

inline bool StartsWithIgnoreCaseAscii(StringSpan str, StringSpan prefix) {
  ASSERT(!prefix.IsEmpty());
  return str.size() >= prefix.size() &&
      EqualsIgnoreCaseAscii(str.GetSlice(0, prefix.size()), prefix);
}

inline bool EndsWithIgnoreCaseAscii(StringSpan str, StringSpan suffix) {
  ASSERT(!suffix.IsEmpty());
  return str.size() >= suffix.size() &&
      EqualsIgnoreCaseAscii(str.GetSlice(str.size() - suffix.size()), suffix);
}

BASE_EXPORT int IndexOfIgnoreCaseAscii(StringSpan str, char c);
BASE_EXPORT int LastIndexOfIgnoreCaseAscii(StringSpan str, char c);

BASE_EXPORT int IndexOfIgnoreCaseAscii(StringSpan haystack, StringSpan needle);
BASE_EXPORT int LastIndexOfIgnoreCaseAscii(StringSpan haystack, StringSpan needle);

BASE_EXPORT void ToLowerAsciiInplace(MutableStringSpan s);
BASE_EXPORT void ToUpperAsciiInplace(MutableStringSpan s);

BASE_EXPORT String ToLowerAscii(StringSpan src) WARN_UNUSED_RESULT;
BASE_EXPORT String ToUpperAscii(StringSpan src) WARN_UNUSED_RESULT;

template<typename T, TEnableIf<TIsStringContainer<T>>* = nullptr>
inline void TrimLeadingWhitespaceAscii(T& str) {
  while (!str.IsEmpty() && IsSpaceAscii(str.GetFirst()))
    str.RemovePrefix(1);
}

template<typename T, TEnableIf<TIsStringContainer<T>>* = nullptr>
inline void TrimTrailingWhitespaceAscii(T& str) {
  while (!str.IsEmpty() && IsSpaceAscii(str.GetLast()))
    str.RemoveSuffix(1);
}

template<typename T, TEnableIf<TIsStringContainer<T>>* = nullptr>
inline void TrimWhitespaceAscii(T& str) {
  // Keep this in this order to minimize copying.
  TrimTrailingWhitespaceAscii(str);
  TrimLeadingWhitespaceAscii(str);
}

template<typename TOutput, typename TInput,
         TEnableIf<TIsStringContainer<TOutput> && TIsStringContainer<TInput>>* = nullptr>
inline void AppendAscii(TOutput& output, const TInput& input) {
  using DstCharType = typename TOutput::ItemType;
  using SrcCharType = typename TInput::ItemType;

  if constexpr (TsAreSame<TRemoveCV<DstCharType>, TRemoveCV<SrcCharType>>) {
    output.Append(input);
  } else {
    int size = input.size();
    auto* dst = output.AppendUninitialized(size);
    for (int i = 0; i < size; ++i)
      dst[i] = char_cast<DstCharType>(input[i]);
  }
}

} // namespace stp

#endif // STP_BASE_TEXT_ASCIISTRING_H_
