// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/AsciiString.h"

#include "Base/Containers/List.h"
#include "Base/Type/Comparable.h"

namespace stp {

namespace detail {

int CompareIgnoreCaseAscii(const char* lhs, const char* rhs, int size) {
  ASSERT(size >= 0);

  for (int i = 0; i < size; ++i) {
    char lc = ToLowerAscii(lhs[i]);
    char rc = ToLowerAscii(rhs[i]);

    int diff = Compare(lc, rc);
    if (diff)
      return diff;
  }
  return 0;
}

} // namespace detail

int IndexOfIgnoreCaseAscii(StringSpan str, char c) {
  c = ToLowerAscii(c);

  for (int i = 0 ; i < str.size(); ++i) {
    if (ToLowerAscii(str[i]) == c)
      return i;
  }
  return -1;
}

int LastIndexOfIgnoreCaseAscii(StringSpan str, char c) {
  c = ToLowerAscii(c);

  for (int i = str.size() - 1; i >= 0; --i) {
    if (ToLowerAscii(str[i]) == c)
      return i;
  }
  return -1;
}

int IndexOfIgnoreCaseAscii(StringSpan haystack, StringSpan needle) {
  if (needle.IsEmpty())
    return 0;

  const char* orig_data = haystack.data();

  char first_char = needle.GetFirst();
  StringSpan needle_rest = needle.GetSlice(1);

  while (haystack.size() >= needle.size()) {
    int found = IndexOfIgnoreCaseAscii(haystack, first_char);
    if (found < 0)
      return -1;

    haystack.RemovePrefix(found + 1);
    if (StartsWithIgnoreCaseAscii(haystack, needle_rest))
      return static_cast<int>(haystack.data() - orig_data - 1);
  }
  return -1;
}

int LastIndexOfIgnoreCaseAscii(StringSpan haystack, StringSpan needle) {
  if (needle.IsEmpty())
    return haystack.size();

  char last_char = needle.GetLast();
  StringSpan needle_rest = needle.GetSlice(0, needle.size() - 1);

  while (haystack.size() >= needle.size()) {
    int found = LastIndexOfIgnoreCaseAscii(haystack, last_char);
    if (found < 0)
      return -1;

    haystack.Truncate(found);
    if (EndsWithIgnoreCaseAscii(haystack, needle_rest))
      return haystack.size() - needle_rest.size();
  }
  return -1;
}

static inline void ToLowerAscii(char* output, const char* input, int length) {
  for (int i = 0; i < length; ++i)
    output[i] = ToLowerAscii(input[i]);
}

static inline void ToUpperAscii(char* output, const char* input, int length) {
  for (int i = 0; i < length; ++i)
    output[i] = ToUpperAscii(input[i]);
}

void ToLowerAsciiInplace(MutableStringSpan s) {
  ToLowerAscii(s.data(), s.data(), s.size());
}
void ToUpperAsciiInplace(MutableStringSpan s) {
  ToUpperAscii(s.data(), s.data(), s.size());
}

String ToLowerAscii(StringSpan src) {
  String rv;
  char* dst = rv.AppendUninitialized(src.size());
  ToLowerAscii(dst, src.data(), src.size());
  return rv;
}

String ToUpperAscii(StringSpan src) {
  String rv;
  char* dst = rv.AppendUninitialized(src.size());
  ToLowerAscii(dst, src.data(), src.size());
  return rv;
}

} // namespace stp
