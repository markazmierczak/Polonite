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

template<class T>
constexpr bool TIsCopyInitializableWithMemset =
    TIsTriviallyDefaultConstructible<T> && sizeof(T) == sizeof(char);

} // namespace detail

template<class T, class U>
constexpr bool areObjectsOverlapping(const T* lhs, int lhs_count, const U* rhs, int rhs_count) noexcept {
  return (rhs <= lhs && lhs < rhs + rhs_count) ||
         (lhs <= rhs && rhs < lhs + lhs_count);
}

template<class T>
constexpr bool areObjectsOverlapping(const T* lhs, const T* rhs, int count) noexcept {
  return areObjectsOverlapping(lhs, count, rhs, count);
}

template<class T>
inline void destroyObjects(T* items, int count) noexcept {
  ASSERT(count >= 0);
  if constexpr (!TIsTriviallyDestructible<T>) {
    for (int i = 0; i < count; ++i)
      destroyObject(items[i]);
  }
}

template<class T>
inline void uninitializedInit(T* items, int count) noexcept(TIsNoexceptConstructible<T>) {
  ASSERT(count >= 0);
  if constexpr (TIsNoexceptConstructible<T>) {
    if constexpr (TIsZeroConstructible<T>) {
      if (count)
        ::memset(items, 0, toUnsigned(count) * sizeof(T));
    } else {
      for (int i = 0; i < count; ++i)
        new (items + i) T();
    }
  } else {
    int i = 0;
    try {
      for (; i < count; ++i)
        new (items + i) T();
    } catch (...) {
      destroyObjects(items, i);
      throw;
    }
  }
}

template<class T>
inline void uninitializedCopy(T* dst, const T* src, int count) noexcept(TIsNoexceptCopyConstructible<T>) {
  ASSERT(count >= 0);
  ASSERT(!areObjectsOverlapping(dst, src, count));
  if constexpr (TIsNoexceptCopyConstructible<T>) {
    if constexpr (TIsTriviallyCopyConstructible<T>) {
      if (count)
        ::memcpy(dst, src, toUnsigned(count) * sizeof(T));
    } else {
      for (int i = 0; i < count; ++i)
        new (dst + i) T(src[i]);
    }
  } else {
    int i = 0;
    try {
      for (; i < count; ++i)
        new (dst + i) T(src[i]);
    } catch(...) {
      destroyObjects(dst, i);
      throw;
    }
  }
}

template<class T>
inline void uninitializedMove(T* dst, T* src, int count) noexcept {
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

template<class T>
inline void uninitializedRelocate(T* dst, T* src, int count) noexcept {
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

template<class T, class U>
inline void uninitializedFill(T* items, int count, U&& value) noexcept(TIsNoexceptCopyConstructible<T>) {
  ASSERT(count >= 0);
  if constexpr (TIsNoexceptCopyConstructible<T>) {
    if constexpr (detail::TIsCopyInitializableWithMemset<T>) {
      if (count)
        ::memset(items, bitCast<uint8_t>(value), toUnsigned(count));
    } else {
      for (int i = 0; i < count; ++i)
        new (items + i) T(value);
    }
  } else {
    ASSERT(items && count >= 0);
    int i = 0;
    try {
      for (; i < count; ++i)
        new (items + i) T(value);
    } catch(...) {
      destroyObjects(items, i);
      throw;
    }
  }
}

template<class T, class U>
inline void fillObjects(T* items, int count, const U& value) noexcept(TIsNoexceptCopyAssignable<T>) {
  ASSERT(count >= 0);
  if constexpr (TIsNoexceptCopyAssignable<T>) {
    if constexpr (detail::TIsCopyInitializableWithMemset<T>) {
      if (count)
        ::memset(items, bitCast<uint8_t>(value), toUnsigned(count));
    } else {
      for (int i = 0; i < count; ++i)
        items[i] = value;
    }
  } else {
    for (int i = 0; i < count; ++i)
      items[i] = value;
  }
}

template<class T>
inline bool equalObjects(const T* lhs, const T* rhs, int count) noexcept {
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

template<class T>
inline void copyObjects(T* dst, const T* src, int count) noexcept(TIsNoexceptCopyAssignable<T>) {
  ASSERT(count >= 0);
  // Destination and source may overlap.
  if constexpr (TIsTriviallyCopyAssignable<T>) {
    if (count)
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

template<class T>
inline void copyObjectsNonOverlapping(T* dst, const T* src, int count) noexcept(TIsNoexceptCopyAssignable<T>) {
  ASSERT(count >= 0);
  ASSERT(!areObjectsOverlapping(dst, src, count));
  if constexpr (TIsTriviallyCopyAssignable<T>) {
    if (count)
      ::memcpy(dst, src, toUnsigned(count) * sizeof(T));
  } else {
    for (int i = 0; i < count; ++i)
      dst[i] = src[i];
  }
}

inline int getLengthOfCString(const char* cstr) noexcept {
  return cstr ? static_cast<int>(::strlen(cstr)) : 0;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_ARRAYOPS_H_
