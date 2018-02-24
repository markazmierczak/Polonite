// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/AsciiString.h"

#include "Base/Containers/List.h"
#include "Base/Math/Alignment.h"
#include "Base/Type/Comparable.h"

namespace stp {

namespace detail {

int compareIgnoringAsciiCase(const char* lhs, const char* rhs, int size) {
  ASSERT(size >= 0);

  for (int i = 0; i < size; ++i) {
    char lc = toLowerAscii(lhs[i]);
    char rc = toLowerAscii(rhs[i]);

    int diff = compare(lc, rc);
    if (diff)
      return diff;
  }
  return 0;
}

} // namespace detail

int compareIgnoringAsciiCase(StringSpan lhs, StringSpan rhs) noexcept {
  int min_size = min(lhs.length(), rhs.length());
  int rv = detail::compareIgnoringAsciiCase(lhs.data(), rhs.data(), min_size);
  return rv != 0 ? rv : compare(lhs.length(), rhs.length());
}

int indexOfIgnoringAsciiCase(StringSpan str, char c) {
  c = toLowerAscii(c);

  for (int i = 0 ; i < str.size(); ++i) {
    if (toLowerAscii(str[i]) == c)
      return i;
  }
  return -1;
}

int lastIndexOfIgnoringAsciiCase(StringSpan str, char c) {
  c = toLowerAscii(c);

  for (int i = str.size() - 1; i >= 0; --i) {
    if (toLowerAscii(str[i]) == c)
      return i;
  }
  return -1;
}

int indexOfIgnoringAsciiCase(StringSpan haystack, StringSpan needle) {
  if (needle.isEmpty())
    return 0;

  const char* orig_data = haystack.data();

  char first_char = needle.getFirst();
  StringSpan needle_rest = needle.getSlice(1);

  while (haystack.size() >= needle.size()) {
    int found = indexOfIgnoringAsciiCase(haystack, first_char);
    if (found < 0)
      return -1;

    haystack.removePrefix(found + 1);
    if (startsWithIgnoringAsciiCase(haystack, needle_rest))
      return static_cast<int>(haystack.data() - orig_data - 1);
  }
  return -1;
}

int lastIndexOfIgnoringAsciiCase(StringSpan haystack, StringSpan needle) {
  if (needle.isEmpty())
    return haystack.size();

  char last_char = needle.getLast();
  StringSpan needle_rest = needle.getSlice(0, needle.size() - 1);

  while (haystack.size() >= needle.size()) {
    int found = lastIndexOfIgnoringAsciiCase(haystack, last_char);
    if (found < 0)
      return -1;

    haystack.truncate(found);
    if (endsWithIgnoringAsciiCase(haystack, needle_rest))
      return haystack.size() - needle_rest.size();
  }
  return -1;
}

bool isAscii(StringSpan text) {
  using MachineWord = uintptr_t;

  MachineWord all_char_bits = 0;

  const char* str = text.data();
  const char* end = str + text.size();

  // Prologue: align the input.
  while (!isAlignedTo(str, isizeof(MachineWord)) && str != end) {
    all_char_bits |= *str;
    ++str;
  }

  // Compare the values of CPU word size.
  constexpr int LoopIncrement = isizeof(MachineWord) / isizeof(char);
  const char* word_end = alignBackward(end, sizeof(MachineWord));
  for (; str < word_end; str += LoopIncrement)
    all_char_bits |= *(reinterpret_cast<const MachineWord*>(str));

  // Process the remaining bytes.
  while (str != end) {
    all_char_bits |= *str;
    ++str;
  }

  constexpr MachineWord NonAsciiBitMask = 0x8080808080808080;
  return !(all_char_bits & NonAsciiBitMask);
}

} // namespace stp
