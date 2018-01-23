// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/Prime.h"

#include "Base/Containers/Array.h"
#include "Base/Containers/BinarySearch.h"

namespace stp {
namespace detail {

static constexpr Array<unsigned, 48> SmallPrimes = {
    0,   2,   3,   5,   7,
   11,  13,  17,  19,  23,
   29,  31,  37,  41,  43,
   47,  53,  59,  61,  67,
   71,  73,  79,  83,  89,
   97, 101, 103, 107, 109,
  113, 127, 131, 137, 139,
  149, 151, 157, 163, 167,
  173, 179, 181, 191, 193,
  197, 199, 211
};

// Potential primes = 210*k + indices[i], k >= 1
// These numbers are not divisible by 2, 3, 5 or 7.
//   (or any integer 2 <= j <= 10 for that matter).
static constexpr Array<unsigned, 48> PrimeIndices = {
    1,  11,  13,  17,  19,  23,
   29,  31,  37,  41,  43,  47,
   53,  59,  61,  67,  71,  73,
   79,  83,  89,  97, 101, 103,
  107, 109, 113, 121, 127, 131,
  137, 139, 143, 149, 151, 157,
  163, 167, 169, 173, 179, 181,
  187, 191, 193, 197, 199, 209
};

template<typename T, int SmallPrimeStartIndex = 0>
static inline bool IsPrimeHelper(T x) {
  if (x <= SmallPrimes.GetLast())
    return BinarySearch(SmallPrimes, static_cast<unsigned>(x)) >= 0;

  // Divide |x| by all primes or potential primes (i) until:
  //    1.  The division is even.
  //    2.  The i > sqrt(n), in which case |x| is prime.
  for (int i = SmallPrimeStartIndex; i < SmallPrimes.size(); ++i) {
    unsigned p = SmallPrimes[i];
    T q = x / p;
    if (q < p)
      return true;
    if (x == q * p)
      return false;
  }

  // |x| was not divisible by small primes, try potential primes.

  static constexpr unsigned Increments[] = {
    0, 10, 2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6, 6, 2,
    6,  4, 2, 6, 4, 6, 8, 4, 2, 4, 2, 4, 8, 6, 4,
    6,  2, 4, 6, 2, 6, 6, 4, 2, 4, 6, 2, 6, 4, 2,
    4,  2, 10,
  };

  T i = 211;
  while (true) {
    for (int j : Increments) {
      i += Increments[j];

      T q = x / i;
      if (q < i)
        return true;
      if (x == q * i)
        return false;
    }
    // This will loop i to the next "plane" of potential primes
    i += 2;
  }
}

bool IsPrimeNumber32(uint32_t x) {
  return IsPrimeHelper(x);
}

bool IsPrimeNumber64(uint64_t x) {
  return IsPrimeHelper(x);
}

#if ASSERT_IS_ON()
static inline bool CheckPrimeOverflow(uint32_t x) {
  return x > UINT32_C(0xFFFFFFFB);
}
static inline bool CheckPrimeOverflow(uint64_t x) {
  return x > UINT64_C(0xFFFFFFFFFFFFFFC5);
}
#endif // ASSERT_IS_ON()

template<typename T>
static inline T NextPrimeHelper(T x) {
  if (x < SmallPrimes.GetLast()) {
    int yi = LowerBound(SmallPrimes, static_cast<unsigned>(x));
    T y = SmallPrimes[yi];
    if (x == y)
      y = SmallPrimes[yi + 1];
    return y;
  }

  ASSERT(!CheckPrimeOverflow(x));
  // Start searching list of potential primes: L * k0 + indices[in]
  const int M = PrimeIndices.size() / sizeof(PrimeIndices[0]);
  // Select first potential prime >= n
  //   Known a-priori n >= L
  const T L = 210;
  T k0 = x / L;
  int in = LowerBound(PrimeIndices, static_cast<unsigned>(x - k0 * L));

  while (true) {
    x = L * k0 + PrimeIndices[in];

    // It is known a-priori that |x| is not divisible by 2, 3, 5 or 7,
    // so don't test those (SmallPrimeStartIndex == 5 ->  divide by 11 first).
    if (IsPrimeHelper<T, 5>(x))
      return x;

    if (++in == M) {
      ++k0;
      in = 0;
    }
  }
}

uint32_t NextPrimeNumber32(uint32_t x) {
  return NextPrimeHelper(x);
}

uint64_t NextPrimeNumber64(uint64_t x) {
  return NextPrimeHelper(x);
}

} // namespace detail
} // namespace stp
