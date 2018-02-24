// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRING_H_
#define STP_BASE_STRING_STRING_H_

#include "Base/String/StringImpl.h"

namespace stp {

class String {
 public:
  String() = default;
  ~String() = default;

  String(String&&) = default;
  String& operator=(String&&) = default;
  String(const String&) = default;
  String& operator=(const String&) = default;

  explicit String(StringSpan text) : impl_(StringImpl::create(text)) {}

  template<int N>
  explicit String(const char (&text)[N]) = delete; // Use StringLiteral instead.

  String(RefPtr<StringImpl> impl) noexcept : impl_(move(impl)) {}

  static String fromCString(const char* cstr);

  operator StringSpan() const { return StringSpan(data(), length()); }

  bool isNull() const noexcept { return !impl_; }
  bool isEmpty() const noexcept { return !impl_ || impl_->isEmpty(); }
  explicit operator bool() const noexcept { return !!impl_; }

  const char* data() const noexcept { return impl_ ? impl_->data() : 0; }
  int length() const noexcept { return impl_ ? impl_->length() : 0; }

  const char& operator[](int at) const noexcept;

  StringImpl* impl() const noexcept { return impl_.get(); }
  RefPtr<StringImpl> releaseImpl() noexcept { return move(impl_); }

  const char& operator[](int at) const noexcept;

  StringSpan substring(int at) const;
  StringSpan substring(int at, int n) const;
  StringSpan left(int n) const { return substring(0, n); }
  StringSpan right(int n) const { return substring(length() - n, n); }

  void truncate(int at);

  void removePrefix(int n);
  void removeSuffix(int n) noexcept { truncate(length() - n); }

  BASE_EXPORT int indexOf(char c) const noexcept;
  int indexOf(char32_t c) const = delete;
  BASE_EXPORT int indexOf(StringSpan s) const noexcept;
  BASE_EXPORT int lastIndexOf(char c) const noexcept;
  int lastIndexOf(char32_t c) const = delete;
  BASE_EXPORT int lastIndexOf(StringSpan s) const noexcept;
  bool contains(char c) const noexcept { return indexOf(c) >= 0; }
  bool contains(char32_t c) const = delete;
  bool contains(StringSpan s) const noexcept { return indexOf(s) >= 0; }

  bool startsWith(StringSpan s) const noexcept;
  bool endsWith(StringSpan s) const noexcept;

  friend void swap(String& x, String& y) noexcept { swap(x.impl_, y.impl_); }

 private:
  RefPtr<StringImpl> impl_;
};

inline String toString(String s) noexcept { return s; }
inline String toString(StringSpan s) { return String(s); }

inline const char& String::operator[](int at) const noexcept {
  ASSERT(impl_ != nullptr);
  return (*impl_)[at];
}

} // namespace stp

#endif // STP_BASE_STRING_STRING_H_
