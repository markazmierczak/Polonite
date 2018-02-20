// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_ALIGNMENT_H_
#define STP_BASE_MATH_ALIGNMENT_H_

#include "Base/Math/PowerOfTwo.h"
#include "Base/Type/Sign.h"

namespace stp {

// Returns true if |value| is a multiple of |alignment|.
// |alignment| must be power of 2.
template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline bool isAlignedTo(T x, T alignment) {
  ASSERT(isPowerOfTwo(alignment));
  return (x & (alignment - 1)) == 0;
}
inline bool isAlignedTo(const void* x, intptr_t alignment) {
  return isAlignedTo(reinterpret_cast<intptr_t>(x), alignment);
}

// Returns alignment of the given |x|.
// Clears all bits except the least significant (returns 1 for x=0).
template<typename T>
inline T whichAlignment(T x) {
  return x ? extractFirstOneBit(x) : 1;
}

inline intptr_t whichAlignment(const void* x) {
  return whichAlignment(reinterpret_cast<intptr_t>(x));
}

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T alignForward(T x, T alignment) {
  ASSERT(isPowerOfTwo(alignment));
  return (x + alignment - 1) & ~(alignment - 1);
}

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
inline T alignBackward(T x, T alignment) {
  ASSERT(isPowerOfTwo(alignment));
  return x & (alignment - 1);
}

// This always succeeds and fills padding with difference in bytes
// between output and input pointer.
template<typename TPointer, typename TInteger>
inline TPointer* alignForward(
    TPointer* pointer, TInteger alignment,
    TInteger* out_padding = nullptr) {
  ASSERT(isPowerOfTwo(alignment));
  uintptr_t uptr = reinterpret_cast<uintptr_t>(pointer);
  uintptr_t aligned_uptr = alignForward<uintptr_t>(uptr, toUnsigned(alignment));
  if (out_padding)
    *out_padding = static_cast<TInteger>(aligned_uptr - uptr);
  return reinterpret_cast<TPointer*>(aligned_uptr);
}

// C++11 std::align implementation.
template<typename TPointer, typename TInteger>
inline bool alignForward(
    TPointer*& pointer, TInteger alignment,
    TInteger size, TInteger& in_out_space,
    TInteger* out_padding = nullptr) {
  ASSERT(isPowerOfTwo(alignment));

  TInteger padding;
  TPointer* result = alignForward(pointer, alignment, &padding);
  if (in_out_space - padding < size)
    return false;

  in_out_space -= padding;
  pointer = result;
  if (out_padding)
    *out_padding = padding;
  return true;
}

template<typename TPointer, typename TInteger>
inline TPointer* alignBackward(TPointer* pointer, TInteger alignment) {
  ASSERT(isPowerOfTwo(alignment));
  uintptr_t ptr = reinterpret_cast<uintptr_t>(pointer);
  uintptr_t aligned_ptr = alignBackward<uintptr_t>(ptr, toUnsigned(alignment));
  return reinterpret_cast<TPointer*>(aligned_ptr);
}

} // namespace stp

#endif // STP_BASE_MATH_ALIGNMENT_H_
