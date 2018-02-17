// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/String16.h"

#include "Base/Debug/Log.h"

namespace stp {

template<typename T>
static inline const T* ToNullTerminatedTmpl(const List<T>& string) {
  ASSERT(!string.Contains('\0'));
  if (string.capacity() != 0) {
    auto* cstr = string.data();
    *(const_cast<T*>(cstr) + string.size()) = '\0';
    return cstr;
  }
  static T NullCharacter = '\0';
  return &NullCharacter;
}

#if SIZEOF_WCHAR_T == 2
WString ToWString(StringSpan input) {
  return ConvertString<wchar_t>(input);
}
WString ToWString(String16Span input) {
  // Safe - we know it will be memcpy'ied.
  return WString(reinterpret_cast<const wchar_t*>(input.data()), input.size());
}
const char16_t* ToNullTerminated(const List<wchar_t>& string) {
  return ToNullTerminatedTmpl(string);
}
#endif

} // namespace stp
