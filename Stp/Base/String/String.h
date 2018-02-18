// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRING_H_
#define STP_BASE_STRING_STRING_H_

#include "Base/String/StringImpl.h"

namespace stp {

class String {
 public:
  // Construct a null string (not an empty string).
  String() = default;
  ~String() = default;

  String(String&&) = default;
  String& operator=(String&&) = default;
  String(const String&) = default;
  String& operator=(const String&) = default;

  BASE_EXPORT explicit String(StringSpan text);

  template<int N> String(const char (&text)[N]) = delete;

  enum CtorFromLiteralTag { CtorFromLiteral };
  // |text| must be null terminated.
  String(CtorFromLiteralTag, const char* text, int length)
      : impl_(StringImpl::createFromLiteral(text, length)) {}

  String(RefPtr<StringImpl> impl) noexcept : impl_(move(impl)) {}

  bool isNull() const noexcept { return !impl_; }
  bool isEmpty() const noexcept { return !impl_ || impl_->isEmpty(); }
  explicit operator bool() const noexcept { return !!impl_; }

  const char* data() const noexcept { return impl_ ? impl_->data() : 0; }
  int size() const noexcept { return impl_ ? impl_->size() : 0; }

  const char& operator[](int at) const noexcept;

  StringImpl* impl() const noexcept { return impl_.get(); }
  RefPtr<StringImpl> releaseImpl() noexcept { return move(impl_); }

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
