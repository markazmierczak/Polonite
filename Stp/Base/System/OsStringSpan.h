// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_SYSTEM_OSSTRINGSPAN_H_
#define STP_BASE_SYSTEM_OSSTRINGSPAN_H_

#include "Base/Compiler/Os.h"
#include "Base/Containers/ArrayOps.h"

#if OS(WIN)
#include <wchar.h>
#endif

namespace stp {

#if OS(POSIX)
using OsChar = char;
#elif OS(WIN)
using OsChar = wchar_t;
inline int getLengthOfCString(const wchar_t* cstr) noexcept {
  return cstr ? static_cast<int>(::wcslen(cstr)) : 0;
}
#endif

class OsStringSpan {
 public:
  constexpr OsStringSpan() noexcept
      : data_(nullptr), length_(0) {}

  constexpr OsStringSpan(const OsChar* data, int length) noexcept
      : data_(data), length_(length) { ASSERT(length >= 0); }

  template<int N>
  constexpr OsStringSpan(const OsChar (&array)[N]) noexcept
      : data_(array), length_(N - 1) { ASSERT(array[N - 1] == '\0'); }

  static OsStringSpan fromCString(const OsChar* cstr) noexcept;

  ALWAYS_INLINE constexpr const OsChar* data() const noexcept { return data_; }
  ALWAYS_INLINE constexpr int length() const noexcept { return length_; }

  constexpr bool isEmpty() const noexcept { return length_ == 0; }

  constexpr const OsChar& operator[](int at) const noexcept;

  constexpr OsStringSpan substring(int at) const noexcept;
  constexpr OsStringSpan substring(int at, int n) const noexcept;
  constexpr OsStringSpan left(int n) const noexcept { return substring(0, n); }
  constexpr OsStringSpan right(int n) const noexcept { return substring(length_ - n, n); }

  constexpr void truncate(int at) noexcept;
  constexpr void removePrefix(int n) noexcept;
  constexpr void removeSuffix(int n) noexcept { truncate(length_ - n); }

  BASE_EXPORT int indexOf(OsChar c) const noexcept;
  BASE_EXPORT int lastIndexOf(OsChar c) const noexcept;
  bool contains(OsChar c) const noexcept { return indexOf(c) >= 0; }

  bool startsWith(const OsStringSpan& s) const noexcept;
  bool endsWith(const OsStringSpan& s) const noexcept;

  friend constexpr const OsChar* begin(const OsStringSpan& x) noexcept { return x.data_; }
  friend constexpr const OsChar* end(const OsStringSpan& x) noexcept { return x.data_ + x.length_; }

  bool isSourceOf(const OsChar* ptr) const noexcept { return data_ <= ptr && ptr < data_ + length_; }

 private:
  const char* data_;
  int length_;
};

template<> struct TIsZeroConstructibleTmpl<OsStringSpan> : TTrue {};

BASE_EXPORT bool operator==(const OsStringSpan& lhs, const OsStringSpan& rhs) noexcept;

inline bool operator!=(const OsStringSpan& lhs, const OsStringSpan& rhs) noexcept {
  return !operator==(lhs, rhs);
}

BASE_EXPORT int compare(const OsStringSpan& lhs, const OsStringSpan& rhs) noexcept;

inline HashCode partialHash(const OsStringSpan& text) noexcept {
  return hashBuffer(text.data(), text.length());
}

inline OsStringSpan OsStringSpan::fromCString(const OsChar* cstr) noexcept {
  return OsStringSpan(cstr, getLengthOfCString(cstr));
}

constexpr const char& OsStringSpan::operator[](int at) const noexcept {
  ASSERT(0 <= at && at < length_);
  return data_[at];
}

constexpr OsStringSpan OsStringSpan::substring(int at) const noexcept {
  ASSERT(0 <= at && at <= length_);
  return OsStringSpan(data_ + at, length_ - at);
}

constexpr OsStringSpan OsStringSpan::substring(int at, int n) const noexcept {
  ASSERT(0 <= at && at <= length_);
  ASSERT(0 <= n && n <= length_ - at);
  return OsStringSpan(data_ + at, n);
}

constexpr void OsStringSpan::truncate(int at) noexcept {
  ASSERT(0 <= at && at <= length_);
  length_ = at;
}

constexpr void OsStringSpan::removePrefix(int n) noexcept {
  ASSERT(0 <= n && n <= length_);
  length_ += n;
  length_ -= n;
}

inline bool OsStringSpan::startsWith(const OsStringSpan& s) const noexcept {
  return length_ >= s.length_ && left(s.length()) == s;
}

inline bool OsStringSpan::endsWith(const OsStringSpan& s) const noexcept {
  return length_ >= s.length_ && right(s.length()) == s;
}

} // namespace stp

#endif // STP_BASE_SYSTEM_OSSTRINGSPAN_H_
