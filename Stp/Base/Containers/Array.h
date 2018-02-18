// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_ARRAY_H_
#define STP_BASE_CONTAINERS_ARRAY_H_

#include "Base/Containers/ArrayFwd.h"
#include "Base/Containers/Span.h"
#include "Base/Type/Common.h"

namespace stp {

template<typename T, int N>
struct Array {
  typedef T ItemType;
  typedef Span<T> SpanType;
  typedef MutableSpan<T> MutableSpanType;

  T data_[N];

  constexpr operator Span<T>() const { return ToSpan(); }
  constexpr operator MutableSpan<T>() { return ToSpan(); }

  ALWAYS_INLINE constexpr const T* data() const { return data_; }
  ALWAYS_INLINE constexpr T* data() { return data_; }
  ALWAYS_INLINE constexpr int size() const { return N; }

  constexpr const T& operator[](int at) const;
  constexpr T& operator[](int at);

  constexpr const T& GetFirst() const { return ToSpan().GetFirst(); }
  constexpr const T& GetLast() const { return ToSpan().GetLast(); }
  constexpr T& GetFirst() { return ToSpan().GetFirst(); }
  constexpr T& GetLast() { return ToSpan().GetLast(); }

  constexpr SpanType GetSlice(int at) const { return ToSpan().GetSlice(at); }
  constexpr SpanType GetSlice(int at, int n) const { return ToSpan().GetSlice(at, n); }
  constexpr MutableSpanType GetSlice(int at) { return ToSpan().GetSlice(at); }
  constexpr MutableSpanType GetSlice(int at, int n) { return ToSpan().GetSlice(at, n); }

  template<typename U>
  int IndexOf(const U& item) const { return IndexOfItem(data_, N, item); }
  template<typename U>
  int LastIndexOf(const U& item) const { return LastIndexOfItem(data_, N, item); }
  template<typename U>
  bool Contains(const U& item) const { return IndexOf(item) >= 0; }

  friend constexpr void swap(Array& lhs, Array& rhs) noexcept { swap(lhs.data_, rhs.data_); }
  friend constexpr bool operator==(const Array& lhs, SpanType rhs) { return operator==(lhs.ToSpan(), rhs); }
  friend constexpr bool operator!=(const Array& lhs, SpanType rhs) { return operator==(lhs.ToSpan(), rhs); }
  friend int Compare(const Array& lhs, SpanType rhs) { return Compare(lhs.ToSpan(), rhs); }

  friend constexpr const T* begin(const Array& x) { return x.data_; }
  friend constexpr const T* end(const Array& x) { return x.data_ + N; }
  friend constexpr T* begin(Array& x) { return x.data_; }
  friend constexpr T* end(Array& x) { return x.data_ + N; }

  friend SpanType MakeSpan(const Array& x) { return x.ToSpan(); }
  friend MutableSpanType MakeSpan(Array& x) { return x.ToSpan(); }

 private:
  constexpr SpanType ToSpan() const { return SpanType(data_, N); }
  constexpr MutableSpanType ToSpan() { return MutableSpanType(data_, N); }
};

template<typename T, int N>
struct TIsZeroConstructibleTmpl<Array<T, N>> : TIsZeroConstructibleTmpl<T> {};
template<typename T, int N>
struct TIsTriviallyRelocatableTmpl<Array<T, N>> : TIsTriviallyRelocatableTmpl<T> {};

template<typename T, int N, TEnableIf<TIsHashable<T>>* = nullptr>
inline HashCode Hash(const Array<T, N>& x) { return HashContiguous(x.data(), x.size()); }

template<typename T, int N, TEnableIf<TIsFormattable<T>>* = nullptr>
inline void Format(TextWriter& out, const Array<T, N>& x, const StringSpan& opts) {
  FormatContiguous(out, x.data(), x.size(), opts);
}
template<typename T, int N, TEnableIf<TIsFormattable<T>>* = nullptr>
inline TextWriter& operator<<(TextWriter& out, const Array<T, N>& x) {
  FormatContiguous(out, x.data(), x.size()); return out;
}

template<typename T = void, typename... TElements>
constexpr auto MakeArray(TElements&&... elements) {
  using DT = TConditional<TIsVoid<T>, TCommon<TElements...>, T>;
  return Array<DT, sizeof...(TElements)> { { Forward<TElements>(elements)... } };
}

template<typename T, int N>
constexpr const T& Array<T, N>::operator[](int at) const {
  ASSERT(0 <= at && at < N);
  return *(data_ + at);
}

template<typename T, int N>
constexpr T& Array<T, N>::operator [](int at) {
  ASSERT(0 <= at && at < N);
  return *(data_ + at);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_ARRAY_H_
