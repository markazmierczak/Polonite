// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_ARRAY_H_
#define STP_BASE_TYPE_ARRAY_H_

#include "Base/Type/CVRef.h"

namespace stp {

namespace detail {

template<typename T>
struct TIsArrayHelper : TFalse {
};

template<typename T, int Size>
struct TIsArrayHelper<T[Size]> : TTrue {};

template<typename T>
struct TIsArrayHelper<T[]> : TTrue {};

template<typename T>
struct TRemoveExtentHelper {
  typedef T Type;
};

template<typename T>
struct TRemoveExtentHelper<T[]> {
  typedef T Type;
};

template<typename T, int N>
struct TRemoveExtentHelper<T[N]> {
  typedef T Type;
};

template<typename T>
struct TRemoveAllExtentsHelper {
  typedef T Type;
};

template<typename T>
struct TRemoveAllExtentsHelper<T[]> {
  typedef typename TRemoveAllExtentsHelper<T>::Type Type;
};

template<typename T, int N>
struct TRemoveAllExtentsHelper<T[N]> {
  typedef typename TRemoveAllExtentsHelper<T>::Type Type;
};

template<typename T>
struct TRankHelper : TIntegerConstant<int, 0> {};

template<typename T>
struct TRankHelper<T[]> : TIntegerConstant<int, TRankHelper<T>::Value + 1> {};

template<typename T, int N>
struct TRankHelper<T[N]> : TIntegerConstant<int, TRankHelper<T>::Value + 1> {};

template<typename T, unsigned N = 0>
struct TExtentHelper : TIntegerConstant<int, 0> {};

template<typename T>
struct TExtentHelper<T[], 0> : TIntegerConstant<int, 0> {};

template<typename T, unsigned I>
struct TExtentHelper<T[], I> : TIntegerConstant<int, TExtentHelper<T, I-1>::Value> {};

template<typename T, int N>
struct TExtentHelper<T[N], 0> : TIntegerConstant<int, N> {};

template<typename T, int N, unsigned I>
struct TExtentHelper<T[N], I> : TIntegerConstant<int, TExtentHelper<T, I-1>::Value> {};

} // namespace detail

template<typename T>
using TRemoveExtent = typename detail::TRemoveExtentHelper<T>::Type;

template<typename T>
using TRemoveAllExtents = typename detail::TRemoveAllExtentsHelper<T>::Type;

// Note: it only checks for C++ array, not Array class.
template<typename T>
constexpr bool TIsArray = detail::TIsArrayHelper<TRemoveCV<T>>::Value;

template<typename T>
constexpr int TRank = detail::TRankHelper<T>::Value;

template<typename T, int N = 0>
constexpr int TExtent = detail::TExtentHelper<T, N>::Value;

template<typename T, int N>
constexpr T* begin(T (&array)[N]) { return array; }
template<typename T, int N>
constexpr T* end(T (&array)[N]) { return array + N; }

} // namespace stp

#endif // STP_BASE_TYPE_ARRAY_H_
