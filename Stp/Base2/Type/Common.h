// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_COMMON_H_
#define STP_BASE_TYPE_COMMON_H_

#include "Base/Type/Variable.h"

namespace stp {

namespace detail {

template<typename ...T>
struct TCommonHelper {};

template<typename T>
struct TCommonHelper<T> {
  typedef typename TCommonHelper<T, T>::Type Type;
};

template<typename T, typename U, typename = void>
struct TCommon2Helper {};

template<typename T, typename U>
struct TCommon2Helper<
    T, U,
    TVoid<decltype(true ? declval<T>() : declval<U>())>> {
  typedef TDecay<decltype(true ? declval<T>() : declval<U>())> Type;
};

template<typename T, typename U,
         typename DT = TDecay<T>,
         typename DU = TDecay<U>>
using TCommon2 =
  TConditional<
    TsAreSame<T, DT> && TsAreSame<U, DU>,
    TCommon2Helper<T, U>,
    TCommonHelper<DT, DU>
  >;

template<typename T, typename U>
struct TCommonHelper<T, U> : TCommon2<T, U> {};

template<typename ...Tp>
struct TCommonN;

template<typename, typename = void>
struct TCommonHelperImpl {};

template<typename T, typename U>
struct TCommonHelperImpl<
    TCommonN<T, U>,
    TVoid<typename TCommonHelper<T, U>::Type>> {
  typedef typename TCommonHelper<T, U>::Type Type;
};

template<typename T, typename U, typename... V>
struct TCommonHelperImpl<TCommonN<T, U, V...>,
    TVoid<typename TCommonHelper<T, U>::Type>>
    : TCommonHelperImpl<
        TCommonN<typename TCommonHelper<T, U>::Type, V...> > {

};

template<typename T, typename U, typename... V>
struct TCommonHelper<T, U, V...> : TCommonHelperImpl<TCommonN<T, U, V...> > {};

} // namespace detail

template<typename... T>
using TCommon = typename detail::TCommonHelper<T...>::Type;

} // namespace stp

#endif // STP_BASE_TYPE_COMMON_H_
