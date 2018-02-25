// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRING_H_
#define STP_BASE_STRING_STRING_H_

#include "Base/String/StringImpl.h"

namespace stp {

class String {
 public:
  String() = default;

  explicit String(const StringSpan& text) : impl_(StringImpl::create(text)) {}

  template<int N>
  explicit String(const char (&text)[N]) = delete; // Use StringLiteral instead.

  String(RefPtr<StringImpl> impl) noexcept : impl_(move(impl)) {}

  static String isolate(String s);

  static String fromCString(const char* cstr);

  StringSpan toSpan() const noexcept { return StringSpan(data(), length()); }
  operator StringSpan() const noexcept { return toSpan(); }

  bool isNull() const noexcept { return !impl_; }
  bool isEmpty() const noexcept { return !impl_ || impl_->isEmpty(); }

  explicit operator bool() const noexcept { return !!impl_; }
  bool operator!() const noexcept { return !!impl_; }

  const char* data() const noexcept { return impl_ ? impl_->data() : 0; }
  int length() const noexcept { return impl_ ? impl_->length() : 0; }

  StringImpl* impl() const noexcept { return impl_.get(); }
  RefPtr<StringImpl> releaseImpl() noexcept { return move(impl_); }

  const char& operator[](int at) const noexcept;

  String substring(int at) const;
  String substring(int at, int n) const;
  String left(int n) const { return substring(0, n); }
  String right(int n) const { return substring(length() - n, n); }

  void truncate(int at);

  void removePrefix(int n);
  void removeSuffix(int n) { truncate(length() - n); }

  int indexOfUnit(char c) const noexcept { return impl_ ? toSpan().indexOfUnit(c) : -1; }
  int lastIndexOfUnit(char c) const noexcept { return impl_ ? toSpan().lastIndexOfUnit(c) : -1; }
  bool containsUnit(char c) const noexcept { return indexOfUnit(c) >= 0; }

  int indexOfRune(char32_t rune) const noexcept { return impl_ ? toSpan().indexOfRune(rune) : -1; }
  int lastIndexOfRune(char32_t rune) const noexcept { return impl_ ? toSpan().lastIndexOfRune(rune) : -1; }
  bool containsRune(char32_t rune) const noexcept { return indexOfRune(rune) >= 0; }

  int indexOf(const StringSpan& s) const noexcept { return impl_ ? toSpan().indexOf(s) : -1; }
  int lastIndexOf(const StringSpan& s) const noexcept { return impl_ ? toSpan().lastIndexOf(s) : -1; }
  bool contains(const StringSpan& s) const noexcept { return indexOf(s) >= 0; }

  bool startsWith(const StringSpan& s) const noexcept { return toSpan().startsWith(s); }
  bool endsWith(const StringSpan& s) const noexcept { return toSpan().endsWith(s); }

  friend void swap(String& x, String& y) noexcept { swap(x.impl_, y.impl_); }
  friend HashCode partialHash(const String& s) noexcept;

 private:
  RefPtr<StringImpl> impl_;
};

BASE_EXPORT bool operator==(const String& lhs, const String& rhs) noexcept;
inline bool operator!=(const String& lhs, const String& rhs) noexcept { return !operator==(lhs, rhs); }
BASE_EXPORT int compare(const String& lhs, String& rhs) noexcept;

#define StringLiteral(text) \
  []() noexcept -> String { \
    static StringImplShape r = { \
      StringImplShape::StaticRefCount, \
      isizeof(text) - 1, text, 0, 0 \
    }; \
    return reinterpret_cast<StaticImpl*>(&r); \
  }

inline String toString(String s) noexcept { return s; }
inline String toString(StringSpan s) { return String(s); }

inline const char& String::operator[](int at) const noexcept {
  ASSERT(impl_ != nullptr);
  return (*impl_)[at];
}

} // namespace stp

#endif // STP_BASE_STRING_STRING_H_
