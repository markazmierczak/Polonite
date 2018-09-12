// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRINGBUILDER_H_
#define STP_BASE_STRING_STRINGBUILDER_H_

#include "Base/String/String.h"

namespace stp {

// TODO be TextWriter
class StringBuilder {
 public:
  StringBuilder() noexcept
      : data_(nullptr), length_(0), capacity_(0) {}

  ~StringBuilder() { destroy(data_); }

  explicit StringBuilder(String&& string) noexcept;

  StringBuilder(StringBuilder&& o) noexcept;
  StringBuilder& operator=(StringBuilder&& o) noexcept;

  void append(char c);
  void append(char16_t rune);
  void append(char32_t rune);

  void append(StringSpan s);
  char* appendUninitialized(int n);

  BASE_EXPORT void clear() noexcept;
  BASE_EXPORT String finish() noexcept;

  char* data() const noexcept { return data_; }
  int length() const noexcept { return length_; }
  int capacity() const noexcept { return capacity_; }

  void reserve(int additional);
  void reserveExact(int additional);
  void shrinkToFit() noexcept;

  const char& operator[](int at) const noexcept;
  char& operator[](int at) noexcept;

 private:
  char* data_;
  int length_;
  int capacity_;

  char* grow(int n);

  static void destroy(char* data) noexcept {
    if (data)
      freeMemory(data);
  }
};

template<> struct TIsZeroConstructibleTmpl<StringBuilder> : TTrue {};
template<> struct TIsTriviallyRelocatableTmpl<StringBuilder> : TTrue {};

inline const char& StringBuilder::operator[](int at) const noexcept {
  ASSERT(0 <= at && at < length_);
  return *(data_ + at);
}

inline char& StringBuilder::operator[](int at) noexcept {
  ASSERT(0 <= at && at < length_);
  return *(data_ + at);
}

} // namespace stp

#endif // STP_BASE_STRING_STRINGBUILDER_H_
