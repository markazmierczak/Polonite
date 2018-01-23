// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_INTEGERSEQUENCE_H_
#define STP_BASE_UTIL_INTEGERSEQUENCE_H_

#include "Base/Compiler/Config.h"
#include "Base/Type/Scalar.h"

namespace stp {

template<typename T, T... Is>
struct IntegerSequence {
  typedef T ValueType;

  static_assert(TIsInteger<T>, "IntegerSequence can only be instantiated with an integral type" );

  static constexpr int size() { return sizeof...(Is); }
};

template<int... Is>
using IndexSequence = IntegerSequence<int, Is...>;

#if !COMPILER(MSVC)
namespace detail {

template<int... Indices>
struct IndexTuple {
  typedef IndexTuple<Indices..., sizeof...(Indices)> Next;
};

template<int N>
struct BuildIndexTuple {
  typedef typename BuildIndexTuple<N - 1>::Type::Next Type;
};

template<>
struct BuildIndexTuple<0> {
  typedef IndexTuple<> Type;
};

template<typename T, T N, typename IS = typename BuildIndexTuple<N>::Type>
struct MakeIntegerSequenceHelper;

template<class T, T N, int... Indices>
struct MakeIntegerSequenceHelper<T, N, IndexTuple<Indices...>> {
  static_assert(TIsInteger<T>, "MakeIntegerSequence can only be instantiated with an integral type" );
  static_assert(N >= 0, "MakeIntegerSequence must have a non-negative sequence size");

  typedef IntegerSequence<T, static_cast<T>(Indices)...> Type;
};

} // namespace detail
#endif // !COMPILER(MSVC)

#if COMPILER(MSVC)
template<typename T, T N>
using MakeIntegerSequence = __make_integer_seq<IntegerSequence, T, N>;
#else
template<typename T, T N>
using MakeIntegerSequence = typename detail::MakeIntegerSequenceHelper<T, N>::Type;
#endif

template<int N>
using MakeIndexSequence = MakeIntegerSequence<int, N>;

template<typename... Ts>
using IndexSequenceFor = MakeIndexSequence<sizeof...(Ts)>;

} // namespace stp

#endif // STP_BASE_UTIL_INTEGERSEQUENCE_H_
