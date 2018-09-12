// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_SYSTEM_OSSTRINGBUF_H_
#define STP_BASE_SYSTEM_OSSTRINGBUF_H_

#include "Base/Memory/Allocate.h"
#include "Base/System/OsStringSpan.h"

namespace stp {

class OsStringBuf {
 public:
  OsStringBuf() noexcept : data_(nullptr), length_(0), capacity_(0) {}
  ~OsStringBuf() { destroy(data_, capacity_); }

  OsStringBuf(OsStringBuf&& o) noexcept;
  OsStringBuf& operator=(OsStringBuf&& o) noexcept;

  BASE_EXPORT OsStringBuf(const OsStringBuf& o);
  OsStringBuf& operator=(const OsStringBuf& o) { return operator=(o.toSpan()); }

  BASE_EXPORT explicit OsStringBuf(OsStringSpan text);
  OsStringBuf& operator=(OsStringSpan o) { assign(o); return *this; }

  // In general you should use StringLiteral instead.
  template<int N> explicit OsStringBuf(const char (&text)[N]) = delete;
  template<int N> OsStringBuf& operator=(const char (&text)[N]) = delete;

  BASE_EXPORT static OsStringBuf fromCString(const char* cstr);
  BASE_EXPORT static OsStringBuf createUninitialized(int length, char*& out_data);

  OsStringSpan toSpan() const noexcept { return StringSpan(data_, length_); }
  operator OsStringSpan() const noexcept { return toSpan(); }

  bool isEmpty() const noexcept { return length_ == 0; }

  const char* data() const noexcept { return data_; }
  int length() const noexcept { return length_; }
  const char* asCString() const noexcept;

  const char& operator[](int at) const noexcept {
    ASSERT(0 <= at && at < length());
    return *(data_ + at);
  }

  OsStringSpan substring(int at) const noexcept { return toSpan().substring(at); }
  OsStringSpan substring(int at, int n) const noexcept { return toSpan().substring(at, n); }
  OsStringSpan left(int n) const noexcept { return toSpan().left(n); }
  OsStringSpan right(int n) const noexcept { return toSpan().right(n); }

  int indexOf(char c) const noexcept { return toSpan().indexOf(c); }
  int lastIndexOf(char c) const noexcept { return toSpan().lastIndexOf(c); }
  bool contains(char c) const noexcept { return indexOf(c) >= 0; }

  int indexOf(char16_t rune) const noexcept { return toSpan().indexOf(rune); }
  int lastIndexOf(char16_t rune) const noexcept { return toSpan().lastIndexOf(rune); }
  bool contains(char16_t rune) const noexcept { return indexOf(rune) >= 0; }

  int indexOf(char32_t rune) const noexcept { return toSpan().indexOf(rune); }
  int lastIndexOf(char32_t rune) const noexcept { return toSpan().lastIndexOf(rune); }
  bool contains(char32_t rune) const noexcept { return indexOf(rune) >= 0; }

  int indexOf(const OsStringSpan& s) const noexcept { return toSpan().indexOf(s); }
  int lastIndexOf(const OsStringSpan& s) const noexcept { return toSpan().lastIndexOf(s); }
  bool contains(const OsStringSpan& s) const noexcept { return indexOf(s) >= 0; }

  bool startsWith(const OsStringSpan& s) const noexcept { return toSpan().startsWith(s); }
  bool endsWith(const OsStringSpan& s) const noexcept { return toSpan().endsWith(s); }

  bool isSourceOf(const char* ptr) const noexcept { return toSpan().isSourceOf(ptr); }

  static OsStringBuf fromLiteral(const char* data, int length) noexcept {
    ASSERT(data[length] == '\0');
    return String(data, length, LiteralCapacity);
  }
  static OsStringBuf fromCStringLiteral(const char* cstr) noexcept {
    return fromLiteral(cstr, getLengthOfCString(cstr));
  }

  static OsStringBuf adoptMemory(const char* data, int length, int capacity) noexcept;
  char* releaseMemory() noexcept;

  friend void swap(OsStringBuf& x, OsStringBuf& y) noexcept {
    swap(x.data_, y.data_);
    swap(x.length_, y.length_);
    swap(x.capacity_, y.capacity_);
  }

  friend const char* begin(const OsStringBuf& x) noexcept { return x.data_; }
  friend const char* end(const OsStringBuf& x) noexcept { return x.data_ + x.length_; }

 private:
  const char* data_;
  int length_;
  int capacity_;

  OsStringBuf(const char* data, int length, int capacity) noexcept
      : data_(data), length_(length), capacity_(capacity) {}

  void assign(OsStringSpan o);

  static void destroy(const char* data, int capacity) noexcept {
    if (capacity > 0)
      freeMemory(const_cast<char*>(data));
  }

  static constexpr int LiteralCapacity = -1;
};

template<> struct TIsZeroConstructibleTmpl<OsStringBuf> : TTrue {};
template<> struct TIsTriviallyRelocatableTmpl<OsStringBuf> : TTrue {};

#define StringLiteral(text) String::fromLiteral(text, isizeof(text) - 1)
#ifndef NDEBUG
#define DebugStringLiteral StringLiteral
#else
#define DebugStringLiteral(text) String()
#endif

inline OsStringBuf toString(OsStringBuf s) noexcept { return s; }
inline OsStringBuf toString(const OsStringSpan& s) { return String(s); }

inline OsStringBuf::OsStringBuf(OsStringBuf&& o) noexcept
    : data_(exchange(o.data_, nullptr)),
      length_(exchange(o.length_, 0)),
      capacity_(exchange(o.capacity_, 0)) {
}

inline OsStringBuf& OsStringBuf::operator=(OsStringBuf&& o) noexcept {
  auto* old_data = exchange(data_, (exchange(o.data_, nullptr)));
  length_ = exchange(o.length_, 0);
  int old_capacity = exchange(capacity_, exchange(o.capacity_, 0));
  destroy(old_data, old_capacity);
  return *this;
}

inline OsStringBuf OsStringBuf::adoptMemory(const char* data, int length, int capacity) noexcept {
  ASSERT(capacity > 0);
  ASSERT(0 <= length && length <= capacity);
  return String(data, length, capacity);
}

inline char* OsStringBuf::releaseMemory() noexcept {
  if (capacity_ <= 0)
    return nullptr;
  length_ = 0;
  capacity_ = 0;
  return const_cast<char*>(exchange(data_, nullptr));
}

inline const char* OsStringBuf::asCString() const noexcept {
  // Check for zero in the middle.
  ASSERT(!data_ || getLengthOfCString(data_) == length_);
  return data_ ? data_ : "";
}

} // namespace stp

#endif // STP_BASE_SYSTEM_OSSTRINGBUF_H_
