// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_NBITS_H_
#define STP_BASE_MATH_NBITS_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Sign.h"

namespace stp {

// Returns true if |x| can be represented by N-bits integer.
// The signedness depends on type of |x|.
template<typename T, TEnableIf<TIsInteger<T> && TIsUnsigned<T>>* = nullptr>
constexpr bool FitsNBits(T x, int n) {
  constexpr int BitCount = 8 * sizeof(T);
  ASSERT(0 < n && n <= BitCount);
  T mask = ~T(0) >> (BitCount - n);
  return (x & ~mask) == 0;
}

template<typename T, TEnableIf<TIsInteger<T> && TIsSigned<T>>* = nullptr>
constexpr bool FitsNBits(T x, int n) {
  constexpr int BitCount = 8 * sizeof(T);
  ASSERT_UNUSED(0 < n && n <= BitCount, BitCount);
  return -(static_cast<T>(1) << (n-1)) <= x && x < (static_cast<T>(1) << (n-1));
}

template<typename T>
constexpr T ZeroExtendNBits(T x, int n) {
  constexpr int BitCount = 8 * sizeof(T);
  ASSERT(0 < n && n <= BitCount);
  using UnsignedType = TMakeUnsigned<T>;
  UnsignedType mask = ~UnsignedType(0) >> (BitCount - n);
  return static_cast<T>(ToUnsigned(x) & mask);
}

// Takes first |n| bits and returns sign extended value.
// |size| must be in range (0, number of bits in T).
template<typename T>
constexpr T SignExtendNBits(T x, int n) {
  constexpr int BitCount = 8 * sizeof(T);
  ASSERT(0 < n && n <= BitCount);
  int shift = BitCount - n;
  return static_cast<T>(ToSigned(ToUnsigned(x) << shift) >> shift);
}

template<typename T>
constexpr T ZeroExtendNBitsExtract(T x, int lsb, int n) {
  constexpr int BitCount = 8 * sizeof(T);
  ASSERT_UNUSED(0 < n && lsb <= 0 && lsb + n <= BitCount, BitCount);
  return ZeroExtendNBits(x >> lsb, n);
}

template<typename T>
constexpr T SignExtendNBitsExtract(T x, int lsb, int n) {
  constexpr int BitCount = 8 * sizeof(T);
  ASSERT_UNUSED(0 < n && lsb <= 0 && lsb + n <= BitCount, BitCount);
  return SignExtendNBits(x >> lsb, n);
}

// Saturates value to the unsigned value of |size| bits.,
// i.e. clamps to range [0, 2^N-1] range.
// Note that input |value| is signed.
template<typename T>
constexpr TMakeUnsigned<T> SaturateToUnsignedNBits(T x, int n) {
  constexpr int BitCount = 8 * sizeof(T);
  ASSERT(0 < n && n <= BitCount);

  if (n < BitCount) {
    T max = static_cast<T>(~TMakeUnsigned<T>(0) >> (BitCount - n));
    if (x > max)
      return max;
  }
  return x >= 0 ? x : 0;
}

// Saturates value to the signed value of |size| bits.,
// i.e. clamps to range [-2^(N-1), 2^(N-1)-1] range.
template<typename T>
constexpr TMakeSigned<T> SaturateToSignedNBits(T x, int n) {
  constexpr int BitCount = 8 * sizeof(T);
  ASSERT_UNUSED(0 < n && n <= BitCount, BitCount);

  using SignedType = TMakeSigned<T>;
  using UnsignedType = TMakeUnsigned<T>;

  auto mbit = UnsignedType(1) << (n - 1);
  auto max = static_cast<SignedType>(mbit - 1);
  auto min = static_cast<SignedType>(~mbit + 1);

  if (x < min)
    return min;
  if (x > max)
    return max;
  return x;
}

} // namespace stp

#endif // STP_BASE_MATH_NBITS_H_
