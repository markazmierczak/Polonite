// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_BUFFERSPAN_H_
#define STP_BASE_CONTAINERS_BUFFERSPAN_H_

#include "Base/Containers/BufferSpanFwd.h"
#include "Base/Containers/Span.h"
#include "Base/Type/Variable.h"

namespace stp {

class BufferSpan {
 public:
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

  template<typename T, TEnableIf<TIsContiguousContainer<T>>* = nullptr>
  explicit constexpr BufferSpan(const T& container) noexcept
      : BufferSpan(container.data(), container.size()) {}

  template<typename T, TEnableIf<TIsTrivial<T>>* = nullptr>
  explicit constexpr operator Span<T>() const noexcept {
    ASSERT(!(ToUnsigned(size_) & (sizeof(T) - 1)));
    return Span<T>(reinterpret_cast<const T*>(data_), static_cast<int>(ToUnsigned(size_) / sizeof(T)));
  }

  ALWAYS_INLINE constexpr const void* data() const { return data_; }
  ALWAYS_INLINE constexpr int size() const { return size_; }

  constexpr bool IsEmpty() const { return size_ == 0; }

  constexpr BufferSpan GetSlice(int at) const {
    ASSERT(0 <= at && at <= size_);
    return BufferSpan(data_ + at, size_ - at);
  }
  constexpr BufferSpan GetSlice(int at, int n) const {
    ASSERT(0 <= at && at <= size_);
    ASSERT(0 <= n && n <= size_ - at);
    return BufferSpan(data_ + at, n);
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

  friend bool operator==(const BufferSpan& lhs, const BufferSpan& rhs) {
    return lhs.size_ == rhs.size_ && CompareData(lhs.data_, rhs.data_, lhs.size_) == 0;
  }
  friend bool operator!=(const BufferSpan& lhs, const BufferSpan& rhs) {
    return !operator==(lhs, rhs);
  }
  friend int Compare(const BufferSpan& lhs, const BufferSpan& rhs) {
    int rv = CompareData(lhs.data_, rhs.data_, lhs.size_ <= rhs.size() ? lhs.size_ : rhs.size());
    return rv ? rv : (lhs.size_ - rhs.size());
  }
  friend HashCode Hash(const BufferSpan& x) { return HashBuffer(x.data_, x.size_); }

  friend void Format(TextWriter& out, const BufferSpan& x, const StringSpan& opts) {
    FormatBuffer(out, x.data_, x.size_, opts);
  }
  friend TextWriter& operator<<(TextWriter& out, const BufferSpan& x) {
    FormatBuffer(out, x.data_, x.size_); return out;
  }

  friend constexpr const void* begin(const BufferSpan& x) { return x.data_; }
  friend constexpr const void* end(const BufferSpan& x) { return x.data_ + x.size_; }

 private:
  const byte_t* data_;
  int size_;

  static int CompareData(const void* lhs, const void* rhs, int count) {
    return count ? ::memcmp(lhs, rhs, ToUnsigned(count)) : 0;
  }
};

class MutableBufferSpan {
 public:
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
      : MutableBufferSpan(array, N) {
    if constexpr (TIsCharacter<T>)
      ASSERT(array[N - 1] == '\0');
  }

  template<typename T, TEnableIf<TIsContiguousContainer<T>>* = nullptr>
  explicit constexpr MutableBufferSpan(const T& container) noexcept
      : MutableBufferSpan(container.data(), container.size()) {}

  constexpr operator BufferSpan() const noexcept { return BufferSpan(data_, size_); }

  template<typename T, TEnableIf<TIsTrivial<T> && TIsConst<T>>* = nullptr>
  explicit constexpr operator Span<T>() const noexcept {
    ASSERT(!(ToUnsigned(size_) & (sizeof(T) - 1)));
    return Span<T>(reinterpret_cast<T*>(data_), static_cast<int>(ToUnsigned(size_) / sizeof(T)));
  }
  template<typename T, TEnableIf<TIsTrivial<T> && !TIsConst<T>>* = nullptr>
  explicit constexpr operator MutableSpan<T>() noexcept {
    ASSERT(!(ToUnsigned(size_) & (sizeof(T) - 1)));
    return MutableSpan<T>(reinterpret_cast<T*>(data_), static_cast<int>(ToUnsigned(size_) / sizeof(T)));
  }

  ALWAYS_INLINE constexpr const void* data() const { return data_; }
  ALWAYS_INLINE constexpr void* data() { return data_; }
  ALWAYS_INLINE constexpr int size() const { return size_; }

  constexpr bool IsEmpty() const { return size_ == 0; }

  constexpr BufferSpan GetSlice(int at) const {
    ASSERT(0 <= at && at <= size_);
    return BufferSpan(data_ + at, size_ - at);
  }
  constexpr MutableBufferSpan GetSlice(int at) {
    ASSERT(0 <= at && at <= size_);
    return MutableBufferSpan(data_ + at, size_ - at);
  }

  constexpr BufferSpan GetSlice(int at, int n) const {
    ASSERT(0 <= at && at <= size_);
    ASSERT(0 <= n && n <= size_ - at);
    return BufferSpan(data_ + at, n);
  }
  constexpr MutableBufferSpan GetSlice(int at, int n) {
    ASSERT(0 <= at && at <= size_);
    ASSERT(0 <= n && n <= size_ - at);
    return MutableBufferSpan(data_ + at, n);
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

  friend bool operator==(const MutableBufferSpan& lhs, const BufferSpan& rhs) {
    return BufferSpan(lhs) == rhs;
  }
  friend bool operator!=(const MutableBufferSpan& lhs, const BufferSpan& rhs) {
    return BufferSpan(lhs) != rhs;
  }
  friend int Compare(const MutableBufferSpan& lhs, const BufferSpan& rhs) {
    return Compare(BufferSpan(lhs), rhs);
  }
  friend HashCode Hash(const MutableBufferSpan& x) { return HashBuffer(x.data_, x.size_); }

  friend void Format(TextWriter& out, const MutableBufferSpan& x, const StringSpan& opts) {
    FormatBuffer(out, x.data_, x.size_, opts);
  }
  friend TextWriter& operator<<(TextWriter& out, const MutableBufferSpan& x) {
    FormatBuffer(out, x.data_, x.size_); return out;
  }

  friend constexpr const void* begin(const MutableBufferSpan& x) { return x.data_; }
  friend constexpr const void* end(const MutableBufferSpan& x) { return x.data_ + x.size_; }
  friend constexpr void* begin(MutableBufferSpan& x) { return x.data_; }
  friend constexpr void* end(MutableBufferSpan& x) { return x.data_ + x.size_; }

 private:
  byte_t* data_;
  int size_;
};

template<>
struct TIsZeroConstructibleTmpl<BufferSpan> : TTrue {};
template<>
struct TIsZeroConstructibleTmpl<MutableBufferSpan> : TTrue {};

template<typename T, TEnableIf<TIsTrivial<T> || TIsVoid<T>>* = nullptr>
constexpr BufferSpan MakeBufferSpan(const T* data, int size) {
  return BufferSpan(data, size);
}
template<typename T, TEnableIf<TIsTrivial<T> || TIsVoid<T>>* = nullptr>
constexpr MutableBufferSpan MakeBufferSpan(T* data, int size) {
  return MutableBufferSpan(data, size);
}

template<typename T, int N, TEnableIf<TIsTrivial<T>>* = nullptr>
constexpr BufferSpan MakeBufferSpan(const T (&array)[N]) {
  return BufferSpan(array);
}
template<typename T, int N, TEnableIf<TIsTrivial<T>>* = nullptr>
constexpr MutableBufferSpan MakeBufferSpan(T (&array)[N]) {
  return MutableBufferSpan(array);
}

template<typename T, TEnableIf<TIsContiguousContainer<T>>* = nullptr>
constexpr auto MakeBufferSpan(T& container) {
  return MakeBufferSpan(container.data(), container.size());
}

inline void Fill(MutableBufferSpan buffer, byte_t byte) {
  if (!buffer.IsEmpty())
    ::memset(buffer.data(), byte, ToUnsigned(buffer.size()));
}

[[nodiscard]] BASE_EXPORT bool TryParse(StringSpan input, MutableBufferSpan output);

} // namespace stp

#endif // STP_BASE_CONTAINERS_BUFFERSPAN_H_