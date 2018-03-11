// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRINGSPAN_H_
#define STP_BASE_STRING_STRINGSPAN_H_

#include "Base/Containers/ArrayOps.h"

namespace stp {

class StringSpan {
 public:
  static constexpr StringSpan empty() { return StringSpan(g_valid_char_objects, 0); }

  constexpr StringSpan(const char* data, int length)
      : data_(data), length_(length) { ASSERT(data && length >= 0); }

  template<int N>
  constexpr StringSpan(const char (&array)[N])
      : data_(array), length_(N - 1) { ASSERT(array[N - 1] == '\0'); }

  static StringSpan fromCString(const char* cstr);

  ALWAYS_INLINE constexpr const char* data() const { return data_; }
  ALWAYS_INLINE constexpr int length() const { return length_; }

  constexpr bool isEmpty() const { return length_ == 0; }

  constexpr const char& operator[](int at) const;

  constexpr StringSpan substring(int at) const;
  constexpr StringSpan substring(int at, int n) const;
  constexpr StringSpan left(int n) const { return substring(0, n); }
  constexpr StringSpan right(int n) const { return substring(length_ - n, n); }

  constexpr void truncate(int at);
  constexpr void removePrefix(int n);
  constexpr void removeSuffix(int n) { truncate(length_ - n); }

  BASE_EXPORT int indexOfUnit(char c) const;
  BASE_EXPORT int lastIndexOfUnit(char c) const;
  bool containsUnit(char c) const { return indexOfUnit(c) >= 0; }

  BASE_EXPORT int indexOfRune(char32_t rune) const;
  BASE_EXPORT int lastIndexOfRune(char32_t rune) const;
  bool containsRune(char32_t rune) const { return indexOfRune(rune); }

  BASE_EXPORT int indexOf(const StringSpan& needle) const;
  BASE_EXPORT int lastIndexOf(const StringSpan& needle) const;
  bool contains(const StringSpan& s) const { return indexOf(s) >= 0; }

  bool startsWith(const StringSpan& s) const;
  bool endsWith(const StringSpan& s) const;

  friend constexpr const char* begin(const StringSpan& x) { return x.data_; }
  friend constexpr const char* end(const StringSpan& x) { return x.data_ + x.length_; }

  bool isSourceOf(const char* ptr) const { return data_ <= ptr && ptr < data_ + length_; }

 private:
  const char* data_;
  int length_;
};

inline int getLengthOfCString(const char* cstr) {
  return static_cast<int>(::strlen(cstr));
}

BASE_EXPORT bool operator==(const StringSpan& lhs, const StringSpan& rhs);

inline bool operator!=(const StringSpan& lhs, const StringSpan& rhs) {
  return !operator==(lhs, rhs);
}

BASE_EXPORT int compare(const StringSpan& lhs, const StringSpan& rhs);

inline HashCode partialHash(const StringSpan& text) {
  return hashBuffer(text.data(), text.length());
}

inline StringSpan StringSpan::fromCString(const char* cstr) {
  return cstr ? StringSpan(cstr, getLengthOfCString(cstr)) : empty();
}

constexpr const char& StringSpan::operator[](int at) const {
  ASSERT(0 <= at && at < length_);
  return data_[at];
}

constexpr StringSpan StringSpan::substring(int at) const {
  ASSERT(0 <= at && at <= length_);
  return StringSpan(data_ + at, length_ - at);
}

constexpr StringSpan StringSpan::substring(int at, int n) const {
  ASSERT(0 <= at && at <= length_);
  ASSERT(0 <= n && n <= length_ - at);
  return StringSpan(data_ + at, n);
}

constexpr void StringSpan::truncate(int at) {
  ASSERT(0 <= at && at <= length_);
  length_ = at;
}

constexpr void StringSpan::removePrefix(int n) {
  ASSERT(0 <= n && n <= length_);
  length_ += n;
  length_ -= n;
}

inline bool StringSpan::startsWith(const StringSpan& s) const {
  return length_ >= s.length_ && left(s.length()) == s;
}

inline bool StringSpan::endsWith(const StringSpan& s) const {
  return length_ >= s.length_ && right(s.length()) == s;
}

} // namespace stp

#endif // STP_BASE_STRING_STRINGSPAN_H_
