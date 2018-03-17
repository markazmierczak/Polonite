// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_ARRAY_H_
#define STP_BASE_CONTAINERS_ARRAY_H_

#include "Base/Containers/Span.h"
#include "Base/Type/Common.h"

namespace stp {

template<class T, int N>
struct Array {
  typedef T ItemType;
  typedef Span<T> SpanType;
  typedef MutableSpan<T> MutableSpanType;
  static constexpr bool IsZeroConstructible = TIsZeroConstructible<T>;
  static constexpr bool IsTriviallyRelocatable = TIsTriviallyRelocatable<T>;

  T data_[N];

  constexpr operator Span<T>() const noexcept { return toSpan(); }
  constexpr operator MutableSpan<T>() noexcept { return toSpan(); }
  constexpr SpanType toSpan() const noexcept { return SpanType(data_, N); }
  constexpr MutableSpanType toSpan() noexcept { return MutableSpanType(data_, N); }

  ALWAYS_INLINE constexpr const T* data() const noexcept { return data_; }
  ALWAYS_INLINE constexpr T* data() noexcept { return data_; }
  ALWAYS_INLINE constexpr int size() const noexcept { return N; }

  constexpr const T& operator[](int at) const noexcept;
  constexpr T& operator[](int at) noexcept;

  constexpr const T& first() const noexcept { return toSpan().first(); }
  constexpr const T& last() const noexcept { return toSpan().last(); }
  constexpr T& first() noexcept { return toSpan().first(); }
  constexpr T& last() noexcept { return toSpan().last(); }

  constexpr SpanType slice(int at) const noexcept { return toSpan().slice(at); }
  constexpr SpanType slice(int at, int n) const noexcept { return toSpan().slice(at, n); }
  constexpr MutableSpanType slice(int at) noexcept { return toSpan().slice(at); }
  constexpr MutableSpanType slice(int at, int n) noexcept { return toSpan().slice(at, n); }

  template<class U> int indexOf(const U& item) const noexcept {
    return toSpan().indexOf(item);
  }
  template<class U> int lastIndexOf(const U& item) const noexcept {
    return toSpan().lastIndexOf(item);
  }
  template<class U> bool contains(const U& item) const noexcept {
    return toSpan().contains(item);
  }

  friend constexpr void swap(Array& lhs, Array& rhs) noexcept {
    swap(lhs.data_, rhs.data_);
  }
  friend constexpr bool operator==(const Array& lhs, SpanType rhs) noexcept {
    return operator==(lhs.toSpan(), rhs);
  }
  friend constexpr bool operator!=(const Array& lhs, SpanType rhs) noexcept {
    return operator==(lhs.toSpan(), rhs);
  }
  friend int compare(const Array& lhs, SpanType rhs) noexcept {
    return compare(lhs.toSpan(), rhs);
  }

  friend constexpr const T* begin(const Array& x) noexcept { return x.data_; }
  friend constexpr const T* end(const Array& x) noexcept { return x.data_ + N; }
  friend constexpr T* begin(Array& x) noexcept { return x.data_; }
  friend constexpr T* end(Array& x) noexcept { return x.data_ + N; }
};

template<class T = void, class... TElements>
constexpr auto makeArray(TElements&&... elements) {
  using DT = TConditional<TIsVoid<T>, TCommon<TElements...>, T>;
  return Array<DT, sizeof...(TElements)> { { forward<TElements>(elements)... } };
}

template<class T, int N>
inline BufferSpan makeBufferSpan(const Array<T, N>& array) noexcept {
  return makeBufferSpan(array.toSpan());
}
template<class T, int N>
inline MutableBufferSpan makeBufferSpan(Array<T, N>& array) noexcept {
  return makeBufferSpan(array.toSpan());
}

template<class T, int N>
constexpr const T& Array<T, N>::operator[](int at) const noexcept {
  ASSERT(0 <= at && at < N);
  return *(data_ + at);
}

template<class T, int N>
constexpr T& Array<T, N>::operator[](int at) noexcept {
  ASSERT(0 <= at && at < N);
  return *(data_ + at);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_ARRAY_H_
