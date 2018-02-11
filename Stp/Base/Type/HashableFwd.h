// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_HASHABLEFWD_H_
#define STP_BASE_TYPE_HASHABLEFWD_H_

#include "Base/Export.h"
#include "Base/Type/Scalar.h"

namespace stp {

template<typename T, TEnableIf<TIsScalar<T>>* = nullptr>
inline HashCode Hash(T x);

namespace detail {

template<typename T>
using THashableConcept = decltype(Hash(declval<const T&>()));

} // namespace detail

template<typename T>
constexpr bool TIsHashable = TsAreSame<HashCode, TDetect<detail::THashableConcept, T>>;

BASE_EXPORT HashCode HashBuffer(const void* data, int size);

template<typename T>
inline HashCode HashContiguous(const T* data, int size);

template<typename T, int N, TEnableIf<TIsHashable<T>>* = nullptr>
inline HashCode Hash(T (&array)[N]) { return HashContiguous(array, N - TIsCharacter<T>); }

} // namespace stp

#endif // STP_BASE_TYPE_HASHABLEFWD_H_
