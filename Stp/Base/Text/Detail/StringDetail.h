// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_DETAIL_STRINGDETAIL_H_
#define STP_BASE_TEXT_DETAIL_STRINGDETAIL_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Sign.h"

#include <string.h>
#include <wchar.h>

namespace stp {
namespace detail {

inline int GetLengthOfCString(const char* str) {
  ASSERT(str);
  return static_cast<int>(::strlen(str));
}

BASE_EXPORT int GetLengthOfCString(const char16_t* str);

BASE_EXPORT int IndexOfAnyCharacter(const char* s, int slength, const char* a, int alength);
BASE_EXPORT int LastIndexOfAnyCharacter(const char* s, int slength, const char* a, int alength);
BASE_EXPORT int IndexOfAnyCharacter(const char16_t* s, int slength, const char16_t* a, int alength);
BASE_EXPORT int LastIndexOfAnyCharacter(const char16_t* s, int slength, const char16_t* a, int alength);

BASE_EXPORT int IndexOfAnyCharacterBut(const char* s, int slength, const char* a, int alength);
BASE_EXPORT int LastIndexOfAnyCharacterBut(const char* s, int slength, const char* a, int alength);

BASE_EXPORT int IndexOfRange(const char* haystack, int hlength, const char* needle, int nlength);
BASE_EXPORT int LastIndexOfRange(const char* haystack, int hlength, const char* needle, int nlength);

BASE_EXPORT bool IsAscii(const char* str, int size);
BASE_EXPORT bool IsAscii(const char16_t* str, int size);

#if SIZEOF_WCHAR_T == 2

inline int GetLengthOfCString(const wchar_t* str) {
  ASSERT(str);
  return static_cast<int>(::wcslen(str));
}

inline void Fill(wchar_t* str, int size, wchar_t c) {
  ASSERT(str && size >= 0);
  ::wmemset(str, c, ToUnsigned(size));
}

inline int IndexOfAnyCharacter(const wchar_t* data, int size, const wchar_t* chars, int nchars) {
  return IndexOfAnyCharacter(
      reinterpret_cast<const char16_t*>(data), size,
      reinterpret_cast<const char16_t*>(chars), nchars);
}
inline int LastIndexOfAnyCharacter(const wchar_t* data, int size, const wchar_t* chars, int nchars) {
  return LastIndexOfAnyCharacter(
      reinterpret_cast<const char16_t*>(data), size,
      reinterpret_cast<const char16_t*>(chars), nchars);
}
inline bool IsAscii(const wchar_t* data, int size) {
  return IsAscii(reinterpret_cast<const char16_t*>(data), size);
}
#endif

} // namespace detail
} // namespace stp

#endif // STP_BASE_TEXT_DETAIL_STRINGDETAIL_H_
