// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_BITS_H_
#define STP_BASE_MATH_BITS_H_

#include "Base/Math/BitsImpl.h"

namespace stp {

// Returns index of first (least-significant) bit that is set.
// -1 is returned if |x| is zero.
template<typename T>
inline int FindFirstOneBit(T x) {
  return detail::FindFirstOneBitImpl(ToUnsigned(x));
}

// Same as above but for last (most-significant) bit.
template<typename T>
inline int FindLastOneBit(T x) {
  return detail::FindLastOneBitImpl(ToUnsigned(x));
}

// Extracts first (least-significant) bit that is set within integral value.
// Note that this returns a bit at the same position
// and not an index as FindFirstOneBit() does.
// 0 is returned if |x| is zero.
template<typename T>
inline T ExtractFirstOneBit(T x) {
  return static_cast<T>(detail::ExtractFirstOneBitImpl(ToUnsigned(x)));
}

// Same as above but for last (most-significant) bit.
template<typename T>
inline T ExtractLastOneBit(T x) {
  using UnsignedType = TMakeUnsigned<T>;
  return x ? static_cast<T>(UnsignedType(1) << FindLastOneBit(x)) : 0;
}

// Returns the number of trailing 0-bits in given integer,
// starting at the least-significant bit position.
//
// If input value is zero, the number of bits in source type is returned.
template<typename T>
inline int CountTrailingZeroBits(T x) {
  return detail::CountTrailingZeroBitsImpl(ToUnsigned(x));
}

// Same as above but for leading (most-significant) 0-bits.
template<typename T>
inline int CountLeadingZeroBits(T x) {
  return detail::CountLeadingZeroBitsImpl(ToUnsigned(x));
}

// Returns true if number of bits set in input value is odd.
template<typename T>
inline bool GetBitsParity(T x) {
  return detail::GetBitsParityImpl(ToUnsigned(x));
}

// Returns the number of bits set in input value.
template<typename T>
inline int CountBitsPopulation(T x) {
  return detail::CountBitsPopulationImpl(ToUnsigned(x));
}

template<typename T>
inline T ReverseBits(T x) {
  return detail::ReverseBitsImpl(ToUnsigned(x));
}

} // namespace stp

#endif // STP_BASE_MATH_BITS_H_
