// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_BITSIMPL_H_
#define STP_BASE_MATH_BITSIMPL_H_

#include "Base/Export.h"
#include "Base/Type/Sign.h"
#include "Base/Compiler/Cpu.h"

#if COMPILER(MSVC)
#include <intrin.h>
#endif

namespace stp {
namespace detail {

BASE_EXPORT int countBitsPopulationImpl(uint32_t x);
BASE_EXPORT int countBitsPopulationImpl(uint64_t x);

BASE_EXPORT extern const uint8_t LookupForBitsReversing[256];

inline uint32_t extractFirstOneBitImpl(uint32_t x) { return x & (~x + 1); }
inline uint64_t extractFirstOneBitImpl(uint64_t x) { return x & (~x + 1); }

#if COMPILER(GCC)
inline int findFirstOneBitImpl(uint32_t x) {
  return x ? __builtin_ctz(x) : -1;
}
inline int findFirstOneBitImpl(uint64_t x) {
  return x ? __builtin_ctzll(x) : -1;
}
#elif COMPILER(MSVC)
inline int findFirstOneBitImpl(uint32_t x) {
  unsigned long result;
  if (_BitScanForward(&result, x))
    return static_cast<int>(result);
  return -1;
}

inline int findFirstOneBitImpl(uint64_t x) {
  #if CPU(ARM_FAMILY) || CPU(X86_64)
  unsigned long result;
  if (_BitScanForward64(&result, x))
    return static_cast<int>(result);
  return -1;
  #else
  uint32_t lo = static_cast<uint32_t>(x >> 0);
  uint32_t hi = static_cast<uint32_t>(x >> 32);

  unsigned long result;
  if (_BitScanForward(&result, lo))
    return static_cast<int>(result);
  if (_BitScanForward(&result, hi))
    return static_cast<int>(result) | 32;
  return -1;
  #endif // CPU(*)
}
#endif // COMPILER(*)

#if COMPILER(GCC)
inline int findLastOneBitImpl(uint32_t x) {
  return x ? (__builtin_clz(x) ^ 31) : -1;
}
inline int findLastOneBitImpl(uint64_t x) {
  return x ? (__builtin_clzll(x) ^ 63) : -1;
}
#elif COMPILER(MSVC)
inline int findLastOneBitImpl(uint32_t x) {
  unsigned long result;
  if (_BitScanReverse(&result, x))
    return static_cast<int>(result);
  return -1;
}

inline int findLastOneBitImpl(uint64_t x) {
  #if CPU(ARM_FAMILY) || CPU(X86_64)
  unsigned long result;
  if (_BitScanReverse64(&result, x))
    return static_cast<int>(result);
  return -1;
  #else
  uint32_t lo = static_cast<uint32_t>(x >> 0);
  uint32_t hi = static_cast<uint32_t>(x >> 32);

  unsigned long result;
  if (_BitScanReverse(&result, hi))
    return static_cast<int>(result) ^ 32;
  return findLastOneBitImpl(lo);
  #endif
}
#endif // COMPILER(*)

#if COMPILER(GCC)
inline int countTrailingZeroBitsImpl(uint32_t x) {
  return x ? __builtin_ctz(x) : 32;
}
inline int countTrailingZeroBitsImpl(uint64_t x) {
  return x ? __builtin_ctzll(x) : 64;
}
#elif COMPILER(MSVC)
inline int countTrailingZeroBitsImpl(uint32_t x) {
  unsigned long result;
  if (_BitScanForward(&result, x))
    return static_cast<int>(result);
  return 32;
}

inline int countTrailingZeroBitsImpl(uint64_t x) {
  #if CPU(ARM_FAMILY) || CPU(X86_64)
  unsigned long result;
  if (_BitScanForward64(&result, x))
    return static_cast<int>(result);
  return 64;
  #else
  uint32_t lo = static_cast<uint32_t>(x >> 0);
  uint32_t hi = static_cast<uint32_t>(x >> 32);

  unsigned long result;
  if (_BitScanForward(&result, lo))
    return static_cast<int>(result);
  return countTrailingZeroBitsImpl(hi) + 32;
  #endif
}
#endif // COMPILER(*)

#if COMPILER(GCC)
inline int countLeadingZeroBitsImpl(uint32_t x) {
  return x ? __builtin_clz(x) : 32;
}
inline int countLeadingZeroBitsImpl(uint64_t x) {
  return x ? __builtin_clzll(x) : 64;
}
#elif COMPILER(MSVC)
inline int countLeadingZeroBitsImpl(uint32_t x) {
  unsigned long result;
  if (_BitScanReverse(&result, x))
    return static_cast<int>(result) ^ 31;
  return 32;
}

inline int countLeadingZeroBitsImpl(uint64_t x) {
  #if CPU(ARM_FAMILY) || CPU(X86_64)
  unsigned long result;
  if (_BitScanReverse64(&result, x))
    return static_cast<unsigned>(result) ^ 63;
  return 64;
  #else
  uint32_t lo = static_cast<uint32_t>(x >> 0);
  uint32_t hi = static_cast<uint32_t>(x >> 32);

  unsigned long result;
  if (_BitScanReverse(&result, hi))
    return static_cast<unsigned>(result) ^ 31;
  return countLeadingZeroBitsImpl(lo) + 32;
  #endif
}
#endif // COMPILER(*)

inline int countLeadingZeroBitsImpl(uint8_t x) {
  return countLeadingZeroBitsImpl(static_cast<uint32_t>(x)) - 24;
}
inline int countLeadingZeroBitsImpl(uint16_t x) {
  return countLeadingZeroBitsImpl(static_cast<uint32_t>(x)) - 16;
}

inline bool getBitsParityImpl(uint32_t x) {
  x ^= x >> 1;
  x ^= x >> 2;
  x = (x & UINT32_C(0x11111111)) * UINT32_C(0x11111111);
  return (x >> 28) & 1;
}

inline bool getBitsParityImpl(uint64_t x) {
  x ^= x >> 1;
  x ^= x >> 2;
  x = (x & UINT64_C(0x1111111111111111)) * UINT64_C(0x1111111111111111);
  return (x >> 60) & 1;
}

#if CPU(ARM_FAMILY)
inline uint32_t reverseBitsImpl(uint32_t x) {
  uint32_t result;
  asm("rbit %[output],%[input]"
      :   [output]  "=r"  (result)
      :   [input]   "r"   (x));
  return result;
}

inline uint8_t reverseBitsImpl(uint8_t x) {
  uint32_t x32 = x;
  return static_cast<uint8_t>(reverseBitsImpl(x32) >> 24);
}
inline uint16_t reverseBitsImpl(uint16_t x) {
  uint32_t x32 = x;
  return static_cast<uint16_t>(reverseBitsImpl(x32) >> 16);
}
#else
inline uint8_t reverseBitsImpl(uint8_t x) {
  return detail::LookupForBitsReversing[x];
}
inline uint16_t reverseBitsImpl(uint16_t x) {
  uint8_t lo = reverseBitsImpl(static_cast<uint8_t>(x >> 8));
  uint8_t hi = reverseBitsImpl(static_cast<uint8_t>(x >> 0));
  return (static_cast<uint16_t>(hi) << 8) | lo;
}
inline uint32_t reverseBitsImpl(uint32_t x) {
  uint16_t lo = reverseBitsImpl(static_cast<uint16_t>(x >> 16));
  uint16_t hi = reverseBitsImpl(static_cast<uint16_t>(x >> 0));
  return (static_cast<uint32_t>(hi) << 16) | lo;
}
#endif // CPU(*)

inline uint64_t reverseBitsImpl(uint64_t x) {
  uint32_t lo = reverseBitsImpl(static_cast<uint32_t>(x >> 32));
  uint32_t hi = reverseBitsImpl(static_cast<uint32_t>(x >> 0));
  return (static_cast<uint64_t>(hi) << 32) | lo;
}

} // namespace detail
} // namespace stp

#endif // STP_BASE_MATH_BITSIMPL_H_
