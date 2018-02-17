// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_STRINGSPAN_H_
#define STP_BASE_TEXT_STRINGSPAN_H_

#include "Base/Containers/Span.h"
#include "Base/Text/Detail/StringDetail.h"

namespace stp {

inline int IndexOfAny(StringSpan list, StringSpan s) {
  return detail::IndexOfAnyCharacter(list.data(), list.size(), s.data(), s.size());
}

inline int LastIndexOfAny(StringSpan list, StringSpan s) {
  return detail::LastIndexOfAnyCharacter(list.data(), list.size(), s.data(), s.size());
}

inline int IndexOfAnyBut(StringSpan list, StringSpan s) {
  return detail::IndexOfAnyCharacterBut(list.data(), list.size(), s.data(), s.size());
}

inline int LastIndexOfAnyBut(StringSpan list, StringSpan s) {
  return detail::LastIndexOfAnyCharacterBut(list.data(), list.size(), s.data(), s.size());
}

} // namespace stp

#endif // STP_BASE_TEXT_STRINGSPAN_H_
