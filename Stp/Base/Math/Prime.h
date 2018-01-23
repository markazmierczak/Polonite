// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_PRIME_H_
#define STP_BASE_MATH_PRIME_H_

#include "Base/Export.h"
#include "Base/Math/SafeConversions.h"
#include "Base/Type/Sign.h"

namespace stp {

namespace detail {

BASE_EXPORT bool IsPrimeNumber32(uint32_t x);
BASE_EXPORT bool IsPrimeNumber64(uint64_t x);

template<typename T, TEnableIf<(sizeof(T) <= 4)>* = nullptr>
inline bool IsPrimeNumberImpl(T x) { return IsPrimeNumber32(x); }
template<typename T, TEnableIf<(sizeof(T) == 8)>* = nullptr>
inline bool IsPrimeNumberImpl(T x) { return IsPrimeNumber64(x); }

BASE_EXPORT uint32_t NextPrimeNumber32(uint32_t x);
BASE_EXPORT uint64_t NextPrimeNumber64(uint64_t x);

template<typename T, TEnableIf<(sizeof(T) <= 4)>* = nullptr>
inline T NextPrimeNumberImpl(T x) { return NextPrimeNumber32(x); }
template<typename T, TEnableIf<(sizeof(T) == 8)>* = nullptr>
inline T NextPrimeNumberImpl(T x) { return NextPrimeNumber64(x); }

} // namespace detail

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T IsPrimeNumber(T x) {
  ASSERT(!IsNegative(x));
  auto ux = ToUnsigned(x);
  return detail::IsPrimeNumberImpl(ux);
}

// Returns the first prime number greater than |x|.
template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T NextPrimeNumber(T x) {
  ASSERT(!IsNegative(x));
  auto ux = ToUnsigned(x);
  auto rv = detail::NextPrimeNumberImpl(ux);
  return CheckedCast<T>(rv);
}

} // namespace stp

#endif // STP_BASE_MATH_PRIME_H_
