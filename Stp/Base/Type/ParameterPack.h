// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_PARAMETERPACK_H_
#define STP_BASE_TYPE_PARAMETERPACK_H_

namespace stp {

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

} // namespace stp

#endif // STP_BASE_TYPE_PARAMETERPACK_H_
