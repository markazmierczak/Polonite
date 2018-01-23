// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/List.h"

#include "Base/Debug/Log.h"
#include "Base/Text/StringUtfConversions.h"

namespace stp {

/** \n List
 * A List is a resizable array (std::vector equivalent).
 * https://en.wikipedia.org/wiki/List_(abstract_data_type)
 */

/** \fn void List::RemoveLast()
 * Removes the last item from the list.
 * The list must not be empty.
 */

/** \fn void List::RemoveAt(int at)
 * Removes an item at the specified index.
 */

/** \fn void List::RemoveRange(int at, int n)
 * Removes a range of items.
 * The given slice must be enclosed by [0..size).
 */

template<typename TDst, typename TSrc>
static inline List<TDst> ConvertString(Span<TSrc> input) {
  List<TDst> output;
  if (!AppendUnicode(output, input))
    LOG(WARN, "replaced illegal sequence with replacement");
  return output;
}

String ToString(String16Span input) {
  return ConvertString<char>(input);
}

String16 ToString16(StringSpan input) {
  return ConvertString<char16_t>(input);
}

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

const char* ToNullTerminated(const List<char>& string) {
  return ToNullTerminatedTmpl(string);
}
const char16_t* ToNullTerminated(const List<char16_t>& string) {
  return ToNullTerminatedTmpl(string);
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
