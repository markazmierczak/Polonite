// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_BITS_H_
#define STP_BASE_MATH_BITS_H_

#include "Base/Math/BitsImpl.h"

namespace stp {

template<typename T>
inline int findFirstOneBit(T x) {
  return detail::findFirstOneBitImpl(toUnsigned(x));
}

template<typename T>
inline int findLastOneBit(T x) {
  return detail::findLastOneBitImpl(toUnsigned(x));
}

template<typename T>
inline T extractFirstOneBit(T x) {
  return static_cast<T>(detail::extractFirstOneBitImpl(toUnsigned(x)));
}

template<typename T>
inline T extractLastOneBit(T x) {
  using UnsignedType = TMakeUnsigned<T>;
  return x ? static_cast<T>(UnsignedType(1) << findLastOneBit(x)) : 0;
}

template<typename T>
inline int countTrailingZeroBits(T x) {
  return detail::countTrailingZeroBitsImpl(toUnsigned(x));
}

template<typename T>
inline int countLeadingZeroBits(T x) {
  return detail::countLeadingZeroBitsImpl(toUnsigned(x));
}

template<typename T>
inline bool getBitsParity(T x) {
  return detail::getBitsParityImpl(toUnsigned(x));
}

template<typename T>
inline int countBitsPopulation(T x) {
  return detail::countBitsPopulationImpl(toUnsigned(x));
}

template<typename T>
inline T reverseBits(T x) {
  return detail::reverseBitsImpl(toUnsigned(x));
}

} // namespace stp

#endif // STP_BASE_MATH_BITS_H_
