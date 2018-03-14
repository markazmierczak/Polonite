// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRING_H_
#define STP_BASE_STRING_STRING_H_

#include "Base/Memory/Allocate.h"
#include "Base/String/StringSpan.h"

namespace stp {

class String {
 public:
  static constexpr bool IsZeroConstructible = true;
  static constexpr bool IsTriviallyRelocatable = true;

  String() noexcept : data_(nullptr), length_(0), capacity_(0) {}
  ~String() { destroy(data_, capacity_); }

  String(String&& o) noexcept;
  String& operator=(String&& o) noexcept;

  BASE_EXPORT String(const String& o);
  String& operator=(const String& o) { return operator=(o.toSpan()); }

  BASE_EXPORT explicit String(StringSpan text);
  String& operator=(StringSpan o) { assign(o); return *this; }

  // In general you should use StringLiteral instead.
  template<int N> explicit String(const char (&text)[N]) = delete;
  template<int N> String& operator=(const char (&text)[N]) = delete;

  BASE_EXPORT static String fromCString(const char* cstr);
  BASE_EXPORT static String createUninitialized(int length, char*& out_data);

  StringSpan toSpan() const noexcept { return StringSpan(data_, length_); }
  operator StringSpan() const noexcept { return toSpan(); }

  bool isEmpty() const noexcept { return length_ == 0; }

  const char* data() const noexcept { return data_; }
  int length() const noexcept { return length_; }
  const char* asCString() const noexcept;

  const char& operator[](int at) const noexcept {
    ASSERT(0 <= at && at < length());
    return *(data_ + at);
  }

  StringSpan substring(int at) const noexcept { return toSpan().substring(at); }
  StringSpan substring(int at, int n) const noexcept { return toSpan().substring(at, n); }
  StringSpan left(int n) const noexcept { return toSpan().left(n); }
  StringSpan right(int n) const noexcept { return toSpan().right(n); }

  int indexOf(char c) const noexcept { return toSpan().indexOf(c); }
  int lastIndexOf(char c) const noexcept { return toSpan().lastIndexOf(c); }
  bool contains(char c) const noexcept { return indexOf(c) >= 0; }

  int indexOf(char16_t rune) const noexcept { return toSpan().indexOf(rune); }
  int lastIndexOf(char16_t rune) const noexcept { return toSpan().lastIndexOf(rune); }
  bool contains(char16_t rune) const noexcept { return indexOf(rune) >= 0; }

  int indexOf(char32_t rune) const noexcept { return toSpan().indexOf(rune); }
  int lastIndexOf(char32_t rune) const noexcept { return toSpan().lastIndexOf(rune); }
  bool contains(char32_t rune) const noexcept { return indexOf(rune) >= 0; }

  int indexOf(const StringSpan& s) const noexcept { return toSpan().indexOf(s); }
  int lastIndexOf(const StringSpan& s) const noexcept { return toSpan().lastIndexOf(s); }
  bool contains(const StringSpan& s) const noexcept { return indexOf(s) >= 0; }

  bool startsWith(const StringSpan& s) const noexcept { return toSpan().startsWith(s); }
  bool endsWith(const StringSpan& s) const noexcept { return toSpan().endsWith(s); }

  bool isSourceOf(const char* ptr) const noexcept { return toSpan().isSourceOf(ptr); }

  static String fromLiteral(const char* data, int length) noexcept {
    ASSERT(data[length] == '\0');
    return String(data, length, LiteralCapacity);
  }
  static String fromCStringLiteral(const char* cstr) noexcept {
    return fromLiteral(cstr, getLengthOfCString(cstr));
  }

  static String adoptMemory(const char* data, int length, int capacity);
  char* releaseMemory();

  friend void swap(String& x, String& y) noexcept {
    swap(x.data_, y.data_);
    swap(x.length_, y.length_);
    swap(x.capacity_, y.capacity_);
  }

  friend const char* begin(const String& x) noexcept { return x.data_; }
  friend const char* end(const String& x) noexcept { return x.data_ + x.length_; }

 private:
  const char* data_;
  int length_;
  int capacity_;

  String(const char* data, int length, int capacity) noexcept
      : data_(data), length_(length), capacity_(capacity) {}

  void assign(StringSpan o);

  static void destroy(const char* data, int capacity) noexcept {
    if (capacity > 0)
      freeMemory(const_cast<char*>(data));
  }

  static constexpr int LiteralCapacity = -1;
};

#define StringLiteral(text) String::fromLiteral(text, isizeof(text) - 1)
#ifndef NDEBUG
#define DebugStringLiteral StringLiteral
#else
#define DebugStringLiteral(text) String()
#endif

inline String toString(String s) noexcept { return s; }
inline String toString(const StringSpan& s) { return String(s); }

inline String::String(String&& o) noexcept
    : data_(exchange(o.data_, nullptr)),
      length_(exchange(o.length_, 0)),
      capacity_(exchange(o.capacity_, 0)) {
}

inline String& String::operator=(String&& o) noexcept {
  auto* old_data = exchange(data_, (exchange(o.data_, nullptr)));
  length_ = exchange(o.length_, 0);
  int old_capacity = exchange(capacity_, exchange(o.capacity_, 0));
  destroy(old_data, old_capacity);
  return *this;
}

inline String String::adoptMemory(const char* data, int length, int capacity) {
  ASSERT(capacity > 0);
  return String(data, length, capacity);
}

inline char* String::releaseMemory() {
  if (capacity_ <= 0)
    return nullptr;
  return const_cast<char*>(exchange(data_, nullptr));
}

inline const char* String::asCString() const noexcept {
  // Check for zero in the middle.
  ASSERT(!data_ || getLengthOfCString(data_) == length_);
  return data_ ? data_ : "";
}

} // namespace stp

#endif // STP_BASE_STRING_STRING_H_
