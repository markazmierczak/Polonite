// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_PARSINGUTIL_H_
#define STP_BASE_TEXT_PARSINGUTIL_H_

namespace stp {

inline bool SkipExactly(const char*& ptr, const char* end, char delimiter) {
  if (ptr < end && *ptr == delimiter) {
    ++ptr;
    return true;
  }
  return false;
}

template<typename TPredicate>
inline bool SkipExactly(const char*& ptr, const char* end, TPredicate&& match) {
  if (ptr < end && match(*ptr)) {
    ++ptr;
    return true;
  }
  return false;
}

inline bool SkipToken(const char*& ptr, const char* end, const char* token) {
  const char* current = ptr;
  while (current < end && *token) {
    if (*current != *token)
      return false;
    ++current;
    ++token;
  }
  if (*token)
    return false;

  ptr = current;
  return true;
}

inline void SkipUntil(const char*& ptr, const char* end, char delimiter) {
  while (ptr < end && *ptr != delimiter)
    ++ptr;
}

template<typename TPredicate>
inline void SkipUntil(const char*& ptr, const char* end, TPredicate&& match) {
  while (ptr < end && !match(*ptr))
    ++ptr;
}

template<typename TPredicate>
inline void SkipWhile(const char*& ptr, const char* end, TPredicate&& match) {
  while (ptr < end && match(*ptr))
    ++ptr;
}

template<typename TPredicate>
inline void ReverseSkipWhile(const char*& ptr, const char* start, TPredicate&& match) {
  while (ptr >= start && match(*ptr))
    --ptr;
}

} // namespace stp

#endif // STP_BASE_TEXT_PARSINGUTIL_H_
