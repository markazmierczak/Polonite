// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_ASCIISTRING_H_
#define STP_BASE_TEXT_ASCIISTRING_H_

#include "Base/String/StringSpan.h"
#include "Base/Text/AsciiChar.h"

namespace stp {

namespace detail {

BASE_EXPORT int compareIgnoringAsciiCase(const char* lhs, const char* rhs, int length);

} // namespace detail

inline bool equalIgnoringAsciiCase(StringSpan lhs, StringSpan rhs) {
  return lhs.length() == rhs.length() &&
      detail::compareIgnoringAsciiCase(lhs.data(), rhs.data(), lhs.length()) == 0;
}

BASE_EXPORT int compareIgnoringAsciiCase(StringSpan lhs, StringSpan rhs);

struct IgnoringAsciiCaseComparer {
  int operator()(StringSpan lhs, StringSpan rhs) const { return compareIgnoringAsciiCase(lhs, rhs); }
};

inline bool startsWithIgnoringAsciiCase(StringSpan str, StringSpan prefix) {
  return str.length() >= prefix.length() &&
      equalIgnoringAsciiCase(str.left(prefix.length()), prefix);
}

inline bool endsWithIgnoringAsciiCase(StringSpan str, StringSpan suffix) {
  return str.length() >= suffix.length() &&
      equalIgnoringAsciiCase(str.right(suffix.length()), suffix);
}

BASE_EXPORT int indexOfIgnoringAsciiCase(const StringSpan& str, char c);
BASE_EXPORT int lastIndexOfIgnoringAsciiCase(const StringSpan& str, char c);

BASE_EXPORT int indexOfIgnoringAsciiCase(StringSpan haystack, StringSpan needle);
BASE_EXPORT int lastIndexOfIgnoringAsciiCase(StringSpan haystack, StringSpan needle);

constexpr int countLeadingSpaceAscii(StringSpan s) {
  int i = 0;
  for (; i < s.length(); ++i) {
    if (!isSpaceAscii(s[i]))
      break;
  }
  return i;
}

constexpr int countTrailingSpaceAscii(StringSpan s) {
  int i = s.length();
  while (i > 0) {
    int next = i - 1;
    if (!isSpaceAscii(s[next]))
      break;
    i = next;
  }
  return s.length() - i;
}

template<typename TString>
inline TString trimWhitespaceAscii(const TString& s) {
  int l = countLeadingSpaceAscii(s);
  int t = countTrailingSpaceAscii(s);
  return s.substring(l, s.length() - l - t);
}

BASE_EXPORT bool isAscii(StringSpan text);

} // namespace stp

#endif // STP_BASE_TEXT_ASCIISTRING_H_
