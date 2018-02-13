// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_STRINGUTFCONVERSIONS_H_
#define STP_BASE_TEXT_STRINGUTFCONVERSIONS_H_

#include "Base/Io/StringWriter.h"
#include "Base/Text/AsciiString.h"
#include "Base/Text/Utf.h"

namespace stp {

template<typename TOutput, TEnableIf<TIsStringContainer<TOutput>>* = nullptr>
inline int AppendRune(TOutput& output, char32_t rune) {
  using CharType = typename TOutput::ItemType;
  using UtfType = UtfTmpl<CharType>;
  if (sizeof(CharType) == 1 && IsAscii(rune)) {
    // Fast path the common case of one byte.
    output.Add(static_cast<char>(rune));
    return 1;
  }
  int count = UtfType::EncodedLength(rune);
  auto* dst = output.AppendUninitialized(count);
  UtfType::Encode(dst, rune);
  return count;
}

namespace detail {

// Guesses the length of the output in code units.
template<typename TOutput, typename TInput>
inline int ComputeSizeHintForAppendUnicode(TOutput& output, const TInput& input) {
  using DstCharType = typename TOutput::ItemType;
  using SrcCharType = typename TInput::ItemType;

  static_assert(sizeof(DstCharType) != sizeof(SrcCharType), "no conversion");

  ASSERT(!input.IsEmpty());

  if (stp::IsAscii(input.GetFirst())) {
    // Assume the input is all ASCII, which means 1:1 correspondence.
    return input.size();
  }
  // Otherwise assume that:
  if (sizeof(typename TOutput::ItemType) == 1) {
    // Assume that the entire input is non-ASCII and will have 3 bytes per char.
    return input.size() * 3;
  }
  if (sizeof(typename TInput::ItemType) == 1) {
    // The UTF-16/32 sequences will have single code unit for each character.
    return input.size() >> 1;
  }
  return input.size();
}

template<typename TOutput, typename TInput>
bool AppendUnicodeNonAscii(TOutput& output, const TInput& input) {
  using DstCharType = typename TOutput::ItemType;
  using SrcCharType = typename TInput::ItemType;
  static_assert(!TsAreSame<TRemoveCV<DstCharType>, TRemoveCV<SrcCharType>>, "!");

  bool all_valid = true;
  auto* src = input.data();
  auto* src_end = src + input.size();
  while (src < src_end) {
    char32_t rune = UtfTmpl<SrcCharType>::Decode(src, src_end);
    if (LIKELY(!UtfBase::IsDecodeError(rune))) {
      AppendRune(output, rune);
    } else {
      AppendRune(output, unicode::ReplacementRune);
      all_valid = false;
    }
  }
  return all_valid;
}

} // namespace detail

template<typename TOutput, typename TInput,
         TEnableIf<TIsStringContainer<TOutput> && TIsStringContainer<TInput>>* = nullptr>
bool AppendUnicode(TOutput& output, const TInput& input) {
  using DstCharType = typename TOutput::ItemType;
  using SrcCharType = typename TInput::ItemType;

  if constexpr (sizeof(DstCharType) == sizeof(SrcCharType)) {
    output.Append(MakeSpan(reinterpret_cast<const DstCharType*>(input.data()), input.size()));
    return true;
  } else {
    if (input.IsEmpty())
      return true;
    if (IsAscii(input)) {
      AppendAscii(output, input);
      return true;
    }
    int size_hint = detail::ComputeSizeHintForAppendUnicode(output, input);
    output.EnsureCapacity(size_hint);
    return detail::AppendUnicodeNonAscii(output, input);
  }
}

} // namespace stp

#endif // STP_BASE_TEXT_STRINGUTFCONVERSIONS_H_
