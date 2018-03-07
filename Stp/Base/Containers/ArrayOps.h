// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_ARRAYOPS_H_
#define STP_BASE_CONTAINERS_ARRAYOPS_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Sign.h"
#include "Base/Type/Variable.h"

#include <new>
#include <string.h>

namespace stp {

namespace detail {

template<typename T>
constexpr bool TIsCopyInitializableWithMemset =
    TIsTriviallyDefaultConstructible<T> && sizeof(T) == sizeof(char);

} // namespace detail

template<typename T, typename U>
constexpr bool areObjectsOverlapping(const T* lhs, int lhs_count, const U* rhs, int rhs_count) {
  return (rhs <= lhs && lhs < rhs + rhs_count) ||
         (lhs <= rhs && rhs < lhs + lhs_count);
}

template<typename T>
constexpr bool areObjectsOverlapping(const T* lhs, const T* rhs, int count) {
  return areObjectsOverlapping(lhs, count, rhs, count);
}

template<typename T>
inline void destroyObjects(T* items, int count) {
  ASSERT(count >= 0);
  if constexpr (!TIsTriviallyDestructible<T>) {
    for (int i = 0; i < count; ++i)
      destroyObject(items[i]);
  }
}

template<typename T>
inline void uninitializedInit(T* items, int count) {
  ASSERT(count >= 0);
  if constexpr (TIsZeroConstructible<T>) {
    if (count)
      ::memset(items, 0, toUnsigned(count) * sizeof(T));
  } else {
    for (int i = 0; i < count; ++i)
      new (items + i) T();
  }
}

template<typename T>
inline void uninitializedCopy(T* dst, const T* src, int count) {
  ASSERT(count >= 0);
  ASSERT(!areObjectsOverlapping(dst, src, count));
  if constexpr (TIsTriviallyCopyConstructible<T>) {
    if (count)
      ::memcpy(dst, src, toUnsigned(count) * sizeof(T));
  } else {
    for (int i = 0; i < count; ++i)
      new (dst + i) T(src[i]);
  }
}

template<typename T>
inline void uninitializedMove(T* dst, T* src, int count) {
  ASSERT(count >= 0);
  ASSERT(!areObjectsOverlapping(dst, src, count));
  if constexpr (TIsTriviallyMoveConstructible<T>) {
    if (count)
      ::memcpy(dst, src, toUnsigned(count) * sizeof(T));
  } else if constexpr (TIsTriviallyRelocatable<T> && TIsZeroConstructible<T>) {
    if (count) {
      auto byte_count = toUnsigned(count) * sizeof(T);
      ::memcpy(dst, src, byte_count);
      ::memset(src, 0, byte_count);
    }
  } else {
    for (int i = 0; i < count; ++i)
      new (dst + i) T(move(src[i]));
  }
}

template<typename T>
inline void uninitializedRelocate(T* dst, T* src, int count) {
  ASSERT(count >= 0);
  // Destination and source may overlap.
  if constexpr (TIsTriviallyRelocatable<T>) {
    if (count)
      ::memmove(dst, src, toUnsigned(count) * sizeof(T));
  } else {
    if (src > dst) {
      for (int i = 0; i < count; ++i) {
        new (dst + i) T(move(src[i]));
        destroyObject(src[i]);
      }
    } else if (src < dst) {
      for (int i = count - 1; i >= 0; --i) {
        new (dst + i) T(move(src[i]));
        destroyObject(src[i]);
      }
    }
  }
}

template<typename T, typename U>
inline void uninitializedFill(T* items, int count, U&& value) {
  ASSERT(count >= 0);
  if constexpr (detail::TIsCopyInitializableWithMemset<T>) {
    if (count)
      ::memset(items, bitCast<uint8_t>(value), toUnsigned(count));
  } else {
    for (int i = 0; i < count; ++i)
      new (items + i) T(value);
  }
}

template<typename T, typename U>
inline void fillObjects(T* items, int count, const U& value) {
  ASSERT(count >= 0);
  if constexpr (detail::TIsCopyInitializableWithMemset<T>) {
    if (count)
      ::memset(items, bitCast<uint8_t>(value), toUnsigned(count));
  } else {
    for (int i = 0; i < count; ++i)
      items[i] = value;
  }
}

template<typename T>
inline bool equalObjects(const T* lhs, const T* rhs, int count) {
  ASSERT(count >= 0);
  if constexpr (TIsTriviallyEqualityComparable<T>) {
    return count ? ::memcmp(lhs, rhs, toUnsigned(count) * sizeof(T)) == 0 : true;
  } else {
    for (int i = 0; i < count; ++i) {
      if (lhs[i] != rhs[i])
        return false;
    }
    return true;
  }
}

template<typename T>
inline void copyObjects(T* dst, const T* src, int count) {
  ASSERT(count >= 0);
  // Destination and source may overlap.
  if constexpr (TIsTriviallyCopyAssignable<T>) {
    if (count != 0)
      ::memmove(dst, src, toUnsigned(count) * sizeof(T));
  } else {
    if (src > dst) {
      for (int i = 0; i < count; ++i)
        dst[i] = src[i];
    } else if (src < dst) {
      for (int i = count - 1; i >= 0; --i)
        dst[i] = src[i];
    }
  }
}

template<typename T>
inline void copyObjectsNonOverlapping(T* dst, const T* src, int count) {
  ASSERT(count >= 0);
  ASSERT(!areObjectsOverlapping(dst, src, count));
  if constexpr (TIsTriviallyCopyAssignable<T>) {
    if (count != 0)
      ::memcpy(dst, src, toUnsigned(count) * sizeof(T));
  } else {
    for (int i = 0; i < count; ++i)
      dst[i] = src[i];
  }
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_ARRAYOPS_H_
