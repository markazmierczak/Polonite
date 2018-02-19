// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_SPAN_H_
#define STP_BASE_CONTAINERS_SPAN_H_

#include "Base/Containers/ArrayOps.h"
#include "Base/Containers/SpanFwd.h"

#include <initializer_list>

namespace stp {

template<typename T>
using InitializerList = std::initializer_list<T>;

template<typename T>
class Span {
 public:
  static_assert(TsAreSame<T, TRemoveCVRef<T>>, "!");

  typedef T ItemType;

  constexpr Span() noexcept
      : data_(nullptr), size_(0) {}

  constexpr Span(const T* data, int size) noexcept
      : data_(data), size_(size) { ASSERT(size >= 0); }

  template<int N>
  constexpr Span(const T (&array)[N]) noexcept
      : data_(array), size_(N - TIsCharacter<T>) {
    if constexpr (TIsCharacter<T>)
      ASSERT(array[N - 1] == '\0');
  }

  constexpr Span(InitializerList<T> ilist) noexcept
      : data_(ilist.begin()), size_(static_cast<int>(ilist.size())) {}

  ALWAYS_INLINE constexpr const T* data() const { return data_; }
  ALWAYS_INLINE constexpr int size() const { return size_; }

  constexpr bool IsEmpty() const { return size_ == 0; }

  constexpr const T& operator[](int at) const {
    ASSERT(0 <= at && at < size_);
    return data_[at];
  }

  constexpr const T& getFirst() const { return operator[](0); }
  constexpr const T& getLast() const { return operator[](size_ - 1); }

  constexpr Span getSlice(int at) const {
    ASSERT(0 <= at && at <= size_);
    return Span(data_ + at, size_ - at);
  }
  constexpr Span getSlice(int at, int n) const {
    ASSERT(0 <= at && at <= size_);
    ASSERT(0 <= n && n <= size_ - at);
    return Span(data_ + at, n);
  }

  constexpr void Truncate(int at) {
    ASSERT(0 <= at && at <= size_);
    size_ = at;
  }

  constexpr void RemovePrefix(int n) {
    ASSERT(0 <= n && n <= size_);
    data_ += n;
    size_ -= n;
  }
  constexpr void RemoveSuffix(int n) { Truncate(size_ - n); }

  template<typename U>
  int indexOf(const U& item) const noexcept;
  template<typename U>
  int lastIndexOf(const U& item) const noexcept;
  template<typename U>
  bool contains(const U& item) const { return indexOf(item) >= 0; }

  friend bool operator==(const Span& lhs, const Span& rhs) {
    return lhs.size_ == rhs.size() && equalObjects(lhs.data_, rhs.data_, lhs.size_);
  }
  friend bool operator!=(const Span& lhs, const Span& rhs) { return !operator==(lhs, rhs); }

  friend constexpr const T* begin(const Span& x) { return x.data_; }
  friend constexpr const T* end(const Span& x) { return x.data_ + x.size_; }

 private:
  const T* data_;
  int size_;
};

template<typename T>
class MutableSpan {
 public:
  static_assert(TsAreSame<T, TRemoveCVRef<T>>, "!");

  typedef T ItemType;

  constexpr MutableSpan() noexcept
      : data_(nullptr), size_(0) {}

  constexpr MutableSpan(T* data, int size) noexcept
      : data_(data), size_(size) { ASSERT(size >= 0); }

  template<int N>
  constexpr MutableSpan(T (&array)[N]) noexcept
      : data_(array), size_(N - TIsCharacter<T>) {
    if constexpr (TIsCharacter<T>)
      ASSERT(array[N - 1] == '\0');
  }

  constexpr operator Span<T>() const noexcept { return Span<T>(data_, size_); }

  ALWAYS_INLINE constexpr const T* data() const { return data_; }
  ALWAYS_INLINE constexpr T* data() { return data_; }
  ALWAYS_INLINE constexpr int size() const { return size_; }

  constexpr bool IsEmpty() const { return size_ == 0; }

  constexpr const T& operator[](int at) const {
    ASSERT(0 <= at && at < size_);
    return data_[at];
  }
  constexpr T& operator[](int at) {
    ASSERT(0 <= at && at < size_);
    return data_[at];
  }

  constexpr const T& getFirst() const { return operator[](0); }
  constexpr const T& getLast() const { return operator[](size_ - 1); }
  constexpr T& getFirst() { return operator[](0); }
  constexpr T& getLast() { return operator[](size_ - 1); }

  constexpr Span<T> getSlice(int at) const {
    ASSERT(0 <= at && at <= size_);
    return Span<T>(data_ + at, size_ - at);
  }
  constexpr MutableSpan getSlice(int at) {
    ASSERT(0 <= at && at <= size_);
    return MutableSpan(data_ + at, size_ - at);
  }

  constexpr Span<T> getSlice(int at, int n) const {
    ASSERT(0 <= at && at <= size_);
    ASSERT(0 <= n && n <= size_ - at);
    return Span<T>(data_ + at, n);
  }
  constexpr MutableSpan getSlice(int at, int n) {
    ASSERT(0 <= at && at <= size_);
    ASSERT(0 <= n && n <= size_ - at);
    return MutableSpan(data_ + at, n);
  }

  constexpr void Truncate(int at) {
    ASSERT(0 <= at && at <= size_);
    size_ = at;
  }

  constexpr void RemovePrefix(int n) {
    ASSERT(0 <= n && n <= size_);
    data_ += n;
    size_ -= n;
  }
  constexpr void RemoveSuffix(int n) { Truncate(size_ - n); }

  template<typename U>
  int indexOf(const U& item) const { return indexOfItem(data_, size_, item); }
  template<typename U>
  int lastIndexOf(const U& item) const { return lastIndexOfItem(data_, size_, item); }
  template<typename U>
  bool contains(const U& item) const { return indexOf(item) >= 0; }

  friend bool operator==(const MutableSpan& lhs, const Span<T>& rhs) {
    return lhs.size_ == rhs.size() && equalObjects(lhs.data_, rhs.data(), lhs.size_);
  }
  friend bool operator!=(const MutableSpan& lhs, const Span<T>& rhs) {
    return !operator==(lhs, rhs);
  }
  friend constexpr const T* begin(const MutableSpan& x) { return x.data_; }
  friend constexpr const T* end(const MutableSpan& x) { return x.data_ + x.size_; }
  friend constexpr T* begin(MutableSpan& x) { return x.data_; }
  friend constexpr T* end(MutableSpan& x) { return x.data_ + x.size_; }

 private:
  T* data_;
  int size_;
};

template<typename T>
struct TIsZeroConstructibleTmpl<Span<T>> : TTrue {};
template<typename T>
struct TIsZeroConstructibleTmpl<MutableSpan<T>> : TTrue {};

template<typename T>
constexpr Span<T> makeSpan(const T* data, int size) { return Span<T>(data, size); }
template<typename T>
constexpr MutableSpan<T> makeSpan(T* data, int size) { return MutableSpan<T>(data, size); }

template<typename T, int N>
constexpr Span<T> makeSpan(const T (&array)[N]) { return array; }
template<typename T, int N>
constexpr MutableSpan<T> makeSpan(T (&array)[N]) { return array; }

template<typename T>
constexpr Span<T> makeSpan(const InitializerList<T>& ilist) { return Span<T>(ilist); }

inline StringSpan makeSpanFromNullTerminated(const char* cstr) {
  ASSERT(cstr);
  return makeSpan(cstr, static_cast<int>(::strlen(cstr)));
}

template<typename T, int N, TEnableIf<TIsCharacter<T>>* = nullptr>
inline const T* ToNullTerminated(const T (&array)[N]) {
  ASSERT(array[N - 1] == '\0');
  return array;
}

template<typename T, int N>
inline bool operator==(const T (&lhs)[N], const Span<T>& rhs) {
  return operator==(makeSpan(lhs), rhs);
}
template<typename T, int N>
inline bool operator==(const T (&lhs)[N], const MutableSpan<T>& rhs) {
  return operator==(makeSpan(lhs), rhs);
}

template<typename T, int N>
inline bool operator!=(const T (&lhs)[N], const Span<T>& rhs) {
  return operator!=(makeSpan(lhs), rhs);
}
template<typename T, int N>
inline bool operator!=(const T (&lhs)[N], const MutableSpan<T>& rhs) {
  return operator!=(makeSpan(lhs), rhs);
}

inline int compare(StringSpan lhs, StringSpan rhs) noexcept {
  if (lhs.IsEmpty() || rhs.IsEmpty())
    return lhs.IsEmpty() == rhs.IsEmpty();
  return ::memcmp(lhs.data(), rhs.data(), toUnsigned(lhs.size()));
}

inline HashCode partialHash(StringSpan text) noexcept { return hashBuffer(text.data(), text.size()); }

template<typename T>
template<typename U>
inline int Span<T>::indexOf(const U& item) const noexcept {
  const T* d = data_;
  for (int i = 0, s = size_; i < s; ++i) {
    if (d[i] == item)
      return i;
  }
  return -1;
}

template<typename T>
template<typename U>
inline int Span<T>::lastIndexOf(const U& item) const noexcept {
  const T* d = data_;
  for (int i = size_ - 1; i >= 0; --i) {
    if (d[i] == item)
      return i;
  }
  return -1;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_SPAN_H_
