// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEM_ADDRESSOF_H_
#define STP_BASE_MEM_ADDRESSOF_H_

#include "Base/Compiler/Config.h"

namespace stp {

#if COMPILER(CLANG) || COMPILER_GCC_AT_LEAST(7, 0)

template<typename T>
constexpr T* AddressOf(T& x) noexcept {
  return __builtin_addressof(x);
}
#else

template <class T>
inline T* AddressOf(T& x) noexcept {
  return reinterpret_cast<T*>(
      const_cast<char*>(&reinterpret_cast<const volatile char&>(x)));
}
#endif // COMPILER(*)

template<typename T>
T* AddressOf(const T&&) noexcept = delete;

} // namespace stp

#endif // STP_BASE_MEM_ADDRESSOF_H_
