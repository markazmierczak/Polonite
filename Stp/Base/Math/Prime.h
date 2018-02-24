// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_PRIME_H_
#define STP_BASE_MATH_PRIME_H_

#include "Base/Math/SafeConversions.h"

namespace stp {

namespace detail {

BASE_EXPORT bool isPrimeNumber32(uint32_t x);
BASE_EXPORT bool isPrimeNumber64(uint64_t x);

template<typename T, TEnableIf<(sizeof(T) <= 4)>* = nullptr>
inline bool isPrimeNumberImpl(T x) { return isPrimeNumber32(x); }
template<typename T, TEnableIf<(sizeof(T) == 8)>* = nullptr>
inline bool isPrimeNumberImpl(T x) { return isPrimeNumber64(x); }

BASE_EXPORT uint32_t nextPrimeNumber32(uint32_t x);
BASE_EXPORT uint64_t nextPrimeNumber64(uint64_t x);

template<typename T, TEnableIf<(sizeof(T) <= 4)>* = nullptr>
inline T nextPrimeNumberImpl(T x) { return nextPrimeNumber32(x); }
template<typename T, TEnableIf<(sizeof(T) == 8)>* = nullptr>
inline T nextPrimeNumberImpl(T x) { return nextPrimeNumber64(x); }

} // namespace detail

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T isPrimeNumber(T x) {
  ASSERT(!isNegative(x));
  auto ux = toUnsigned(x);
  return detail::isPrimeNumberImpl(ux);
}

// Returns the first prime number greater than |x|.
template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T nextPrimeNumber(T x) {
  ASSERT(!isNegative(x));
  auto ux = toUnsigned(x);
  auto rv = detail::nextPrimeNumberImpl(ux);
  return assertedCast<T>(rv);
}

} // namespace stp

#endif // STP_BASE_MATH_PRIME_H_
