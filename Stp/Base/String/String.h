// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRING_H_
#define STP_BASE_STRING_STRING_H_

#include "Base/String/StringImpl.h"

namespace stp {

class String {
 public:
  String(String&& other) noexcept : impl_(move(other.impl_)) {}
  String(const String& other) noexcept : impl_(other.impl_.copyRef()) {}

  String& operator=(String&& other) noexcept { impl_ = move(other.impl_); return *this; }
  String& operator=(const String& other) noexcept { impl_ = other.impl_.copyRef(); return *this; }

  explicit String(const StringSpan& text) : impl_(StringImpl::create(text)) {}

  template<int N>
  explicit String(const char (&text)[N]) = delete; // Use StringLiteral instead.

  String(Rc<StringImpl> impl) noexcept : impl_(move(impl)) {}

  static String isolate(String s);

  BASE_EXPORT static String fromCString(const char* cstr);
  static String empty() noexcept { return adoptRc(StringImpl::staticEmpty()); }

  static String createUninitialized(int length, char*& out_data);

  StringSpan toSpan() const noexcept { return impl_->toSpan(); }
  operator StringSpan() const noexcept { return impl_->toSpan(); }

  bool isEmpty() const noexcept { return impl_->isEmpty(); }

  const char* data() const noexcept { return impl_->data(); }
  int length() const noexcept { return impl_->length(); }

  StringImpl& getImpl() const noexcept { return impl_; }

  const char& operator[](int at) const noexcept {
    ASSERT(0 <= at && at < length());
    return *(data() + at);
  }

  String substring(int at) const { return substring(at, length() - at); }
  String substring(int at, int n) const { return impl_->substring(at, n); }
  String left(int n) const { return substring(0, n); }
  String right(int n) const { return substring(length() - n, n); }

  int indexOfUnit(char c) const noexcept { return toSpan().indexOfUnit(c); }
  int lastIndexOfUnit(char c) const noexcept { return toSpan().lastIndexOfUnit(c); }
  bool containsUnit(char c) const noexcept { return indexOfUnit(c) >= 0; }

  int indexOfRune(char32_t rune) const noexcept { return toSpan().indexOfRune(rune); }
  int lastIndexOfRune(char32_t rune) const noexcept { return toSpan().lastIndexOfRune(rune); }
  bool containsRune(char32_t rune) const noexcept { return indexOfRune(rune) >= 0; }

  int indexOf(const StringSpan& s) const noexcept { return toSpan().indexOf(s); }
  int lastIndexOf(const StringSpan& s) const noexcept { return toSpan().lastIndexOf(s); }
  bool contains(const StringSpan& s) const noexcept { return indexOf(s) >= 0; }

  bool startsWith(const StringSpan& s) const noexcept { return toSpan().startsWith(s); }
  bool endsWith(const StringSpan& s) const noexcept { return toSpan().endsWith(s); }

  friend void swap(String& x, String& y) noexcept { swap(x.impl_, y.impl_); }
  friend HashCode partialHash(const String& s) noexcept;

 private:
  Rc<StringImpl> impl_;
};

BASE_EXPORT bool operator==(const String& lhs, const String& rhs) noexcept;
inline bool operator!=(const String& lhs, const String& rhs) noexcept { return !operator==(lhs, rhs); }
BASE_EXPORT int compare(const String& lhs, String& rhs) noexcept;

#define StringLiteral(text) \
  []() noexcept -> String { \
    static StaticStringImpl<isizeof(text)> r = { \
      { StringImplShape::StaticRefCount, isizeof(text) - 1, StringImplShape::NotInterned }, \
      text \
    }; \
    return reinterpret_cast<StringImpl&>(r.shape); \
  }

inline String toString(String s) noexcept { return s; }
inline String toString(const StringSpan& s) { return String(s); }

inline String String::createUninitialized(int length, char*& out_data) {
  return StringImpl::createUninitialized(length, out_data);
}

} // namespace stp

#endif // STP_BASE_STRING_STRING_H_
