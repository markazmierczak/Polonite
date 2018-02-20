// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_ASCIISTRING_H_
#define STP_BASE_TEXT_ASCIISTRING_H_

#include "Base/Containers/ListFwd.h"
#include "Base/Containers/Span.h"
#include "Base/Text/AsciiChar.h"

namespace stp {

namespace detail {

BASE_EXPORT int compareIgnoreCaseAscii(const char* lhs, const char* rhs, int size);

} // namespace detail

inline bool equalIgnoreCaseAscii(StringSpan lhs, StringSpan rhs) {
  return lhs.size() == rhs.size() &&
      detail::compareIgnoreCaseAscii(lhs.data(), rhs.data(), lhs.size()) == 0;
}

inline int compareIgnoreCaseAscii(StringSpan lhs, StringSpan rhs) {
  int min_size = min(lhs.size(), rhs.size());
  int rv = detail::compareIgnoreCaseAscii(lhs.data(), rhs.data(), min_size);
  return rv != 0 ? rv : (lhs.size() - rhs.size());
}

struct IgnoreCaseAsciiComparer {
  int operator()(StringSpan lhs, StringSpan rhs) const { return compareIgnoreCaseAscii(lhs, rhs); }
};

inline bool startsWithIgnoreCaseAscii(StringSpan str, StringSpan prefix) {
  ASSERT(!prefix.isEmpty());
  return str.size() >= prefix.size() &&
      equalIgnoreCaseAscii(str.getSlice(0, prefix.size()), prefix);
}

inline bool endsWithIgnoreCaseAscii(StringSpan str, StringSpan suffix) {
  ASSERT(!suffix.isEmpty());
  return str.size() >= suffix.size() &&
      equalIgnoreCaseAscii(str.getSlice(str.size() - suffix.size()), suffix);
}

BASE_EXPORT int indexOfIgnoreCaseAscii(StringSpan str, char c);
BASE_EXPORT int lastIndexOfIgnoreCaseAscii(StringSpan str, char c);

BASE_EXPORT int indexOfIgnoreCaseAscii(StringSpan haystack, StringSpan needle);
BASE_EXPORT int lastIndexOfIgnoreCaseAscii(StringSpan haystack, StringSpan needle);

BASE_EXPORT void toLowerAsciiInplace(MutableStringSpan s);
BASE_EXPORT void toUpperAsciiInplace(MutableStringSpan s);

BASE_EXPORT String toLowerAscii(StringSpan src) WARN_UNUSED_RESULT;
BASE_EXPORT String toUpperAscii(StringSpan src) WARN_UNUSED_RESULT;

template<typename TString>
inline void trimLeadingSpaceAscii(TString& str) {
  while (!str.isEmpty() && isSpaceAscii(str.getFirst()))
    str.removePrefix(1);
}

template<typename TString>
inline void trimTrailingSpaceAscii(TString& str) {
  while (!str.isEmpty() && isSpaceAscii(str.getLast()))
    str.removeSuffix(1);
}

template<typename TString>
inline void trimSpaceAscii(TString& str) {
  // Keep this in this order to minimize copying.
  trimTrailingSpaceAscii(str);
  trimLeadingSpaceAscii(str);
}

BASE_EXPORT bool isAscii(StringSpan text);

} // namespace stp

#endif // STP_BASE_TEXT_ASCIISTRING_H_
