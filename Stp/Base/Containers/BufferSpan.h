// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_BUFFERSPAN_H_
#define STP_BASE_CONTAINERS_BUFFERSPAN_H_

#include "Base/Containers/ArrayOps.h"

namespace stp {

BASE_EXPORT void formatBuffer(TextWriter& out, const void* data, int size, const StringSpan& opts);
BASE_EXPORT void formatBuffer(TextWriter& out, const void* data, int size);

class BufferSpan {
 public:
  static constexpr bool IsZeroConstructible = true;

  constexpr BufferSpan() noexcept
      : data_(nullptr), size_(0) {}

  template<typename T, TEnableIf<TIsVoid<T>>* = nullptr>
  explicit constexpr BufferSpan(const T* data, int size) noexcept
      : data_(static_cast<const byte_t*>(data)), size_(size) {}

  template<typename T, TEnableIf<TIsTrivial<T>>* = nullptr>
  explicit constexpr BufferSpan(const T* data, int size) noexcept
      : data_(reinterpret_cast<const byte_t*>(data)), size_(size * isizeof(T)) {}

  template<typename T, int N, TEnableIf<TIsTrivial<T>>* = nullptr>
  explicit constexpr BufferSpan(const T (&array)[N]) noexcept
      : BufferSpan(array, N - TIsCharacter<T>) {
    if constexpr (TIsCharacter<T>)
      ASSERT(array[N - 1] == '\0');
  }

  ALWAYS_INLINE constexpr const void* data() const noexcept { return data_; }
  ALWAYS_INLINE constexpr int size() const noexcept { return size_; }

  constexpr bool isEmpty() const noexcept { return size_ == 0; }

  constexpr BufferSpan slice(int at) const noexcept {
    ASSERT(0 <= at && at <= size_);
    return BufferSpan(data_ + at, size_ - at);
  }
  constexpr BufferSpan slice(int at, int n) const noexcept {
    ASSERT(0 <= at && at <= size_);
    ASSERT(0 <= n && n <= size_ - at);
    return BufferSpan(data_ + at, n);
  }

  constexpr void truncate(int at) noexcept {
    ASSERT(0 <= at && at <= size_);
    size_ = at;
  }

  constexpr void removePrefix(int n) noexcept {
    ASSERT(0 <= n && n <= size_);
    data_ += n;
    size_ -= n;
  }
  constexpr void removeSuffix(int n) noexcept { truncate(size_ - n); }

  friend bool operator==(const BufferSpan& lhs, const BufferSpan& rhs) noexcept {
    return lhs.size_ == rhs.size_ && compareData(lhs.data_, rhs.data_, lhs.size_) == 0;
  }
  friend bool operator!=(const BufferSpan& lhs, const BufferSpan& rhs) noexcept {
    return !operator==(lhs, rhs);
  }
  friend int compare(const BufferSpan& lhs, const BufferSpan& rhs) noexcept {
    int rv = compareData(lhs.data_, rhs.data_, lhs.size_ <= rhs.size() ? lhs.size_ : rhs.size());
    return rv ? rv : (lhs.size_ - rhs.size());
  }
  friend HashCode partialHash(const BufferSpan& x) noexcept { return hashBuffer(x.data_, x.size_); }

  friend void format(TextWriter& out, const BufferSpan& x, const StringSpan& opts) noexcept {
    formatBuffer(out, x.data_, x.size_, opts);
  }
  friend TextWriter& operator<<(TextWriter& out, const BufferSpan& x) noexcept {
    formatBuffer(out, x.data_, x.size_); return out;
  }

  friend constexpr const void* begin(const BufferSpan& x) noexcept { return x.data_; }
  friend constexpr const void* end(const BufferSpan& x) noexcept { return x.data_ + x.size_; }

 private:
  const byte_t* data_;
  int size_;

  static int compareData(const void* lhs, const void* rhs, int count) noexcept {
    return count ? ::memcmp(lhs, rhs, toUnsigned(count)) : 0;
  }
};

class MutableBufferSpan {
 public:
  static constexpr bool IsZeroConstructible = true;

  constexpr MutableBufferSpan() noexcept
      : data_(nullptr), size_(0) {}

  template<typename T, TEnableIf<TIsVoid<T> && !TIsConst<T>>* = nullptr>
  explicit constexpr MutableBufferSpan(T* data, int size) noexcept
      : data_(static_cast<byte_t*>(data)), size_(size) {}

  template<typename T, TEnableIf<TIsTrivial<T> && !TIsConst<T>>* = nullptr>
  explicit constexpr MutableBufferSpan(T* data, int size) noexcept
      : data_(reinterpret_cast<byte_t*>(data)), size_(size * isizeof(T)) {}

  template<typename T, int N, TEnableIf<TIsTrivial<T> && !TIsConst<T>>* = nullptr>
  explicit constexpr MutableBufferSpan(T (&array)[N]) noexcept
      : MutableBufferSpan(array, N - TIsCharacter<T>) {
    if constexpr (TIsCharacter<T>)
      ASSERT(array[N - 1] == '\0');
  }

  constexpr operator BufferSpan() const noexcept { return BufferSpan(data_, size_); }

  ALWAYS_INLINE constexpr const void* data() const noexcept { return data_; }
  ALWAYS_INLINE constexpr void* data() noexcept { return data_; }
  ALWAYS_INLINE constexpr int size() const noexcept { return size_; }

  constexpr bool isEmpty() const noexcept { return size_ == 0; }

  constexpr BufferSpan slice(int at) const noexcept {
    ASSERT(0 <= at && at <= size_);
    return BufferSpan(data_ + at, size_ - at);
  }
  constexpr MutableBufferSpan slice(int at) noexcept {
    ASSERT(0 <= at && at <= size_);
    return MutableBufferSpan(data_ + at, size_ - at);
  }

  constexpr BufferSpan slice(int at, int n) const noexcept {
    ASSERT(0 <= at && at <= size_);
    ASSERT(0 <= n && n <= size_ - at);
    return BufferSpan(data_ + at, n);
  }
  constexpr MutableBufferSpan slice(int at, int n) noexcept {
    ASSERT(0 <= at && at <= size_);
    ASSERT(0 <= n && n <= size_ - at);
    return MutableBufferSpan(data_ + at, n);
  }

  constexpr void truncate(int at) noexcept {
    ASSERT(0 <= at && at <= size_);
    size_ = at;
  }

  constexpr void removePrefix(int n) noexcept {
    ASSERT(0 <= n && n <= size_);
    data_ += n;
    size_ -= n;
  }
  constexpr void removeSuffix(int n) noexcept { truncate(size_ - n); }

  void fill(byte_t byte) noexcept;

  friend bool operator==(const MutableBufferSpan& lhs, const BufferSpan& rhs) noexcept {
    return BufferSpan(lhs) == rhs;
  }
  friend bool operator!=(const MutableBufferSpan& lhs, const BufferSpan& rhs) noexcept {
    return BufferSpan(lhs) != rhs;
  }
  friend int compare(const MutableBufferSpan& lhs, const BufferSpan& rhs) noexcept {
    return compare(BufferSpan(lhs), rhs);
  }
  friend HashCode partialHash(const MutableBufferSpan& x) noexcept {
    return hashBuffer(x.data_, x.size_);
  }

  friend void format(TextWriter& out, const MutableBufferSpan& x, const StringSpan& opts) {
    formatBuffer(out, x.data_, x.size_, opts);
  }
  friend TextWriter& operator<<(TextWriter& out, const MutableBufferSpan& x) {
    formatBuffer(out, x.data_, x.size_); return out;
  }

  friend constexpr const void* begin(const MutableBufferSpan& x) noexcept { return x.data_; }
  friend constexpr const void* end(const MutableBufferSpan& x) noexcept { return x.data_ + x.size_; }
  friend constexpr void* begin(MutableBufferSpan& x) noexcept { return x.data_; }
  friend constexpr void* end(MutableBufferSpan& x) noexcept { return x.data_ + x.size_; }

 private:
  byte_t* data_;
  int size_;
};

template<typename T, TEnableIf<TIsTrivial<T> || TIsVoid<T>>* = nullptr>
constexpr BufferSpan makeBufferSpan(const T* data, int size) noexcept {
  return BufferSpan(data, size);
}
template<typename T, TEnableIf<TIsTrivial<T> || TIsVoid<T>>* = nullptr>
constexpr MutableBufferSpan makeBufferSpan(T* data, int size) noexcept {
  return MutableBufferSpan(data, size);
}

template<typename T, int N, TEnableIf<TIsTrivial<T>>* = nullptr>
constexpr BufferSpan makeBufferSpan(const T (&array)[N]) noexcept {
  return BufferSpan(array);
}
template<typename T, int N, TEnableIf<TIsTrivial<T>>* = nullptr>
constexpr MutableBufferSpan makeBufferSpan(T (&array)[N]) noexcept {
  return MutableBufferSpan(array);
}

inline void MutableBufferSpan::fill(byte_t byte) noexcept {
  if (!isEmpty())
    ::memset(data_, byte, toUnsigned(size_));
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_BUFFERSPAN_H_
