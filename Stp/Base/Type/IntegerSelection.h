// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_INTEGERSELECTION_H_
#define STP_BASE_TYPE_INTEGERSELECTION_H_

#include "Base/Type/Basic.h"

namespace stp {

namespace detail {

template<bool TIsSigned, int TSize>
struct TMakeIntegerHelper;

template<>
struct TMakeIntegerHelper<true, 1> {
  typedef int8_t Type;
};

template<>
struct TMakeIntegerHelper<false, 1> {
  typedef uint8_t Type;
};

template<>
struct TMakeIntegerHelper<true, 2> {
  typedef int16_t Type;
};

template<>
struct TMakeIntegerHelper<false, 2> {
  typedef uint16_t Type;
};

template<>
struct TMakeIntegerHelper<true, 4> {
  typedef int32_t Type;
};

template<>
struct TMakeIntegerHelper<false, 4> {
  typedef uint32_t Type;
};

template<>
struct TMakeIntegerHelper<true, 8> {
  typedef int64_t Type;
};

template<>
struct TMakeIntegerHelper<false, 8> {
  typedef uint64_t Type;
};

} // namespace detail

template<bool TIsSigned, int TSize>
using TMakeInteger = typename detail::TMakeIntegerHelper<TIsSigned, TSize>::Type;

} // namespace stp

#endif // STP_BASE_TYPE_INTEGERSELECTION_H_
