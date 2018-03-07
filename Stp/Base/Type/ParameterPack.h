// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_PARAMETERPACK_H_
#define STP_BASE_TYPE_PARAMETERPACK_H_

#include "Base/Compiler/Config.h"

namespace stp {

#if COMPILER(CLANG)
template<int N, typename... Ts>
using TNthTypeFromParameterPack = __type_pack_element<N, Ts...>;
#else
namespace detail {

template<int N, typename... Ts>
struct TNthTypeFromParameterPackHelper;

template<int N, typename T, typename... OtherTs>
struct TNthTypeFromParameterPackHelper<N, T, OtherTs...> {
  typedef typename TNthTypeFromParameterPackHelper<N - 1, OtherTs...>::Type Type;
};

template<typename T, typename... OtherTs>
struct TNthTypeFromParameterPackHelper<0, T, OtherTs...> {
  typedef T Type;
};

} // namespace detail

template<int N, typename... Ts>
using TNthTypeFromParameterPack = typename detail::TNthTypeFromParameterPackHelper<N, Ts...>::Type;

#endif // COMPILER(*)

} // namespace stp

#endif // STP_BASE_TYPE_PARAMETERPACK_H_
