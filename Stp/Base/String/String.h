// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRING_H_
#define STP_BASE_STRING_STRING_H_

#include "Base/Memory/Allocate.h"
#include "Base/String/StringSpan.h"

namespace stp {

class String {
 public:
  String() : data_(nullptr), length_(0), capacity_(0) {}

  ~String() {
    if (data_)
      freeMemory(const_cast<char*>(data_));
  }

  String(String&& o);
  String& operator=(String&& o) { exchange(*this, move(o)); return *this;  }

  BASE_EXPORT explicit String(const String& o);
  String& operator=(const String& o) { return operator=(o.toSpan()); }

  BASE_EXPORT explicit String(StringSpan text);
  String& operator=(StringSpan o) { assign(o); return *this; }

  // In general you should use StringLiteral instead.
  template<int N> explicit String(const char (&text)[N]) = delete;
  template<int N> String& operator=(const char (&text)[N]) = delete;

  BASE_EXPORT static String fromCString(const char* cstr);

  BASE_EXPORT static String createUninitialized(int length, char*& out_data);

  StringSpan toSpan() const { return StringSpan(data_, length_); }
  operator StringSpan() const { return toSpan(); }

  bool isEmpty() const { return length_ == 0; }

  const char* data() const { return data_; }
  int length() const { return length_; }
  const char* asCString() const { return data_ ? data_ : ""; }

  const char& operator[](int at) const {
    ASSERT(0 <= at && at < length());
    return *(data_ + at);
  }

  StringSpan substring(int at) const { return toSpan().substring(at); }
  StringSpan substring(int at, int n) const { return toSpan().substring(at, n); }
  StringSpan left(int n) const { return toSpan().left(n); }
  StringSpan right(int n) const { return toSpan().right(n); }

  int indexOfUnit(char c) const { return toSpan().indexOfUnit(c); }
  int lastIndexOfUnit(char c) const { return toSpan().lastIndexOfUnit(c); }
  bool containsUnit(char c) const { return indexOfUnit(c) >= 0; }

  int indexOfRune(char32_t rune) const { return toSpan().indexOfRune(rune); }
  int lastIndexOfRune(char32_t rune) const { return toSpan().lastIndexOfRune(rune); }
  bool containsRune(char32_t rune) const { return indexOfRune(rune) >= 0; }

  int indexOf(const StringSpan& s) const { return toSpan().indexOf(s); }
  int lastIndexOf(const StringSpan& s) const { return toSpan().lastIndexOf(s); }
  bool contains(const StringSpan& s) const { return indexOf(s) >= 0; }

  bool startsWith(const StringSpan& s) const { return toSpan().startsWith(s); }
  bool endsWith(const StringSpan& s) const { return toSpan().endsWith(s); }

  bool isSourceOf(const char* ptr) const { return toSpan().isSourceOf(ptr); }

  friend void swap(String& x, String& y) {
    swap(x.data_, y.data_);
    swap(x.length_, y.length_);
    swap(x.capacity_, y.capacity_);
  }
  friend HashCode partialHash(const String& s);

  static String fromLiteral(const char* data, int length) {
    ASSERT(data[length] == '\0');
    return String(data, length, LiteralCapacity);
  }
  static String fromCStringLiteral(const char* cstr) {
    return fromLiteral(cstr, lengthOfCString(cstr));
  }

  friend const char* begin(const String& x) { return x.data_; }
  friend const char* end(const String& x) { return x.data_ + x.length_; }

 private:
  const char* data_;
  int length_;
  int capacity_;

  String(const char* data, int length, int capacity)
      : data_(data), length_(length), capacity_(capacity) {}

  void assign(StringSpan o);

  static constexpr int LiteralCapacity = -1;
};

#define StringLiteral(text) String::fromLiteral(text, isizeof(text))
#ifndef NDEBUG
#define DebugStringLiteral StringLiteral
#else
#define DebugStringLiteral(text) String()
#endif

inline String toString(String s) { return s; }
inline String toString(const StringSpan& s) { return String(s); }

inline String::String(String&& o)
    : data_(exchange(o.data_, nullptr)),
      length_(exchange(o.length_, 0)),
      capacity_(exchange(o.capacity_, 0)) {
}

} // namespace stp

#endif // STP_BASE_STRING_STRING_H_
