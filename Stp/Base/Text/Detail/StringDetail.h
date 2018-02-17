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

BASE_EXPORT int IndexOfAnyCharacter(const char* s, int slength, const char* a, int alength);
BASE_EXPORT int LastIndexOfAnyCharacter(const char* s, int slength, const char* a, int alength);

BASE_EXPORT int IndexOfAnyCharacterBut(const char* s, int slength, const char* a, int alength);
BASE_EXPORT int LastIndexOfAnyCharacterBut(const char* s, int slength, const char* a, int alength);

BASE_EXPORT int IndexOfRange(const char* haystack, int hlength, const char* needle, int nlength);
BASE_EXPORT int LastIndexOfRange(const char* haystack, int hlength, const char* needle, int nlength);

#if SIZEOF_WCHAR_T == 2
inline int GetLengthOfCString(const wchar_t* str) {
  ASSERT(str);
  return static_cast<int>(::wcslen(str));
}
#endif

} // namespace detail
} // namespace stp

#endif // STP_BASE_TEXT_DETAIL_STRINGDETAIL_H_
