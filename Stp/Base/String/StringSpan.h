// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRINGSPAN_H_
#define STP_BASE_STRING_STRINGSPAN_H_

#include "Base/Containers/ArrayOps.h"

namespace stp {

class StringSpan {
 public:
  static constexpr bool IsZeroConstructible = true;

  constexpr StringSpan() noexcept
      : data_(nullptr), length_(0) {}

  constexpr StringSpan(const char* data, int length) noexcept
      : data_(data), length_(length) { ASSERT(length >= 0); }

  template<int N>
  constexpr StringSpan(const char (&array)[N]) noexcept
      : data_(array), length_(N - 1) { ASSERT(array[N - 1] == '\0'); }

  static StringSpan fromCString(const char* cstr) noexcept;

  ALWAYS_INLINE constexpr const char* data() const noexcept { return data_; }
  ALWAYS_INLINE constexpr int length() const noexcept { return length_; }

  constexpr bool isEmpty() const noexcept { return length_ == 0; }

  constexpr const char& operator[](int at) const noexcept;

  constexpr StringSpan substring(int at) const noexcept;
  constexpr StringSpan substring(int at, int n) const noexcept;
  constexpr StringSpan left(int n) const noexcept { return substring(0, n); }
  constexpr StringSpan right(int n) const noexcept { return substring(length_ - n, n); }

  constexpr void truncate(int at) noexcept;
  constexpr void removePrefix(int n) noexcept;
  constexpr void removeSuffix(int n) noexcept { truncate(length_ - n); }

  BASE_EXPORT int indexOf(char c) const noexcept;
  BASE_EXPORT int lastIndexOf(char c) const noexcept;
  bool contains(char c) const noexcept { return indexOf(c) >= 0; }

  int indexOf(char16_t rune) const noexcept { return indexOf(static_cast<char32_t>(rune)); }
  int lastIndexOf(char16_t rune) const noexcept { return lastIndexOf(static_cast<char32_t>(rune)); }
  bool contains(char16_t rune) const noexcept { return indexOf(rune) >= 0; }

  BASE_EXPORT int indexOf(char32_t rune) const noexcept;
  BASE_EXPORT int lastIndexOf(char32_t rune) const noexcept;
  bool contains(char32_t rune) const noexcept { return indexOf(rune) >= 0; }

  BASE_EXPORT int indexOf(const StringSpan& needle) const noexcept;
  BASE_EXPORT int lastIndexOf(const StringSpan& needle) const noexcept;
  bool contains(const StringSpan& s) const noexcept { return indexOf(s) >= 0; }

  bool startsWith(const StringSpan& s) const noexcept;
  bool endsWith(const StringSpan& s) const noexcept;

  friend constexpr const char* begin(const StringSpan& x) noexcept { return x.data_; }
  friend constexpr const char* end(const StringSpan& x) noexcept { return x.data_ + x.length_; }

  bool isSourceOf(const char* ptr) const noexcept { return data_ <= ptr && ptr < data_ + length_; }

 private:
  const char* data_;
  int length_;
};

BASE_EXPORT bool operator==(const StringSpan& lhs, const StringSpan& rhs) noexcept;

inline bool operator!=(const StringSpan& lhs, const StringSpan& rhs) noexcept {
  return !operator==(lhs, rhs);
}

BASE_EXPORT int compare(const StringSpan& lhs, const StringSpan& rhs) noexcept;

inline HashCode partialHash(const StringSpan& text) noexcept {
  return hashBuffer(text.data(), text.length());
}

inline StringSpan StringSpan::fromCString(const char* cstr) noexcept {
  return StringSpan(cstr, getLengthOfCString(cstr));
}

constexpr const char& StringSpan::operator[](int at) const noexcept {
  ASSERT(0 <= at && at < length_);
  return data_[at];
}

constexpr StringSpan StringSpan::substring(int at) const noexcept {
  ASSERT(0 <= at && at <= length_);
  return StringSpan(data_ + at, length_ - at);
}

constexpr StringSpan StringSpan::substring(int at, int n) const noexcept {
  ASSERT(0 <= at && at <= length_);
  ASSERT(0 <= n && n <= length_ - at);
  return StringSpan(data_ + at, n);
}

constexpr void StringSpan::truncate(int at) noexcept {
  ASSERT(0 <= at && at <= length_);
  length_ = at;
}

constexpr void StringSpan::removePrefix(int n) noexcept {
  ASSERT(0 <= n && n <= length_);
  length_ += n;
  length_ -= n;
}

inline bool StringSpan::startsWith(const StringSpan& s) const noexcept {
  return length_ >= s.length_ && left(s.length()) == s;
}

inline bool StringSpan::endsWith(const StringSpan& s) const noexcept {
  return length_ >= s.length_ && right(s.length()) == s;
}

} // namespace stp

#endif // STP_BASE_STRING_STRINGSPAN_H_
