// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_SWAPBYTES_H_
#define STP_BASE_UTIL_SWAPBYTES_H_

#include "Base/Compiler/Config.h"
#include "Base/Type/Sign.h"

#include <limits.h>

#if COMPILER(MSVC)
#include <stdlib.h>
#endif

namespace stp {

inline unsigned char swapBytes(unsigned char x) { return x; }

inline unsigned short swapBytes(unsigned short x) {
  #if COMPILER(MSVC)
  return _byteswap_ushort(x);
  #else
  return __builtin_bswap16(x);
  #endif
}

inline unsigned int swapBytes(unsigned int x) {
  #if COMPILER(MSVC)
  return _byteswap_ulong(x);
  #else
  return __builtin_bswap32(x);
  #endif
}

inline unsigned long long swapBytes(unsigned long long x) {
  #if COMPILER(MSVC)
  return _byteswap_uint64(x);
  #else
  return __builtin_bswap64(x);
  #endif
}

inline unsigned long swapBytes(unsigned long x) {
  #if ULONG_MAX == UINT_MAX
  return static_cast<unsigned long>(swapBytes(static_cast<unsigned int>(x)));
  #elif ULONG_MAX == ULLONG_MAX
  return static_cast<unsigned long>(swapBytes(static_cast<unsigned long long>(x)));
  #endif
}

template<typename T, TEnableIf<TIsSigned<T>>* = nullptr>
inline T swapBytes(T x) {
  return static_cast<T>(swapBytes(toUnsigned(x)));
}

} // namespace stp

#endif // STP_BASE_UTIL_SWAPBYTES_H_
