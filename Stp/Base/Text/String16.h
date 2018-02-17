// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_STRING16_H_
#define STP_BASE_TEXT_STRING16_H_

#include "Base/Containers/List.h"

#include <wchar.h>

namespace stp {

inline String16 ToString16(String16 s) { return s; }
inline String16 ToString16(String16Span s) { return String16(s); }
BASE_EXPORT String16 ToString16(StringSpan s);

BASE_EXPORT String ToString(String16Span s);

BASE_EXPORT const char16_t* ToNullTerminated(const List<char16_t>& string);

#if SIZEOF_WCHAR_T == 2
inline int GetLengthOfCString(const wchar_t* str) {
  ASSERT(str);
  return static_cast<int>(::wcslen(str));
}
#endif

#if SIZEOF_WCHAR_T == 2
inline WString ToWString(WString s) { return s; }
inline WString ToWString(WStringSpan s) { return WString(s); }

BASE_EXPORT String ToString(WStringSpan s);
BASE_EXPORT String16 ToString16(WStringSpan s);
BASE_EXPORT WString ToWString(StringSpan s);
BASE_EXPORT WString ToWString(String16Span s);
BASE_EXPORT const wchar_t* ToNullTerminated(const List<wchar_t>& string);
#endif

} // namespace stp

#endif // STP_BASE_TEXT_STRING16_H_
