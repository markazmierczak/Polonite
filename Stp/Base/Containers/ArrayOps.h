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
constexpr bool AreOverlapping(const T* lhs, int lhs_count, const U* rhs, int rhs_count) noexcept {
  return (rhs <= lhs && lhs < rhs + rhs_count) ||
         (lhs <= rhs && rhs < lhs + lhs_count);
}

template<typename T>
constexpr bool AreOverlapping(const T* lhs, const T* rhs, int count) noexcept {
  return AreOverlapping(lhs, count, rhs, count);
}

template<typename T>
inline void DestroyAt(T* item) noexcept {
  ASSERT(item != nullptr);
  item->~T();
}

template<typename T>
inline void Destroy(T* items, int count) noexcept {
  ASSERT(count >= 0);
  if constexpr (!TIsTriviallyDestructible<T>) {
    for (int i = 0; i < count; ++i)
      DestroyAt(items + i);
  }
}

template<typename T>
inline void UninitializedInit(T* items, int count) noexcept(TIsNoexceptConstructible<T>) {
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
      Destroy(items, i);
      throw;
    }
  }
}

template<typename T>
inline void UninitializedCopy(T* dst, const T* src, int count) noexcept(TIsNoexceptCopyConstructible<T>) {
  ASSERT(count >= 0);
  ASSERT(!AreOverlapping(dst, src, count));
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
      Destroy(dst, i);
      throw;
    }
  }
}

template<typename T>
inline void UninitializedMove(T* dst, T* src, int count) noexcept {
  ASSERT(count >= 0);
  ASSERT(!AreOverlapping(dst, src, count));
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
inline void UninitializedRelocate(T* dst, T* src, int count) noexcept {
  ASSERT(count >= 0);
  // Destination and source may overlap.
  if constexpr (TIsTriviallyRelocatable<T>) {
    if (count)
      ::memmove(dst, src, toUnsigned(count) * sizeof(T));
  } else {
    if (src > dst) {
      for (int i = 0; i < count; ++i) {
        new (dst + i) T(move(src[i]));
        DestroyAt(src + i);
      }
    } else if (src < dst) {
      for (int i = count - 1; i >= 0; --i) {
        new (dst + i) T(move(src[i]));
        DestroyAt(src + i);
      }
    }
  }
}

template<typename T, typename U>
inline void UninitializedFill(T* items, int count, U&& value) noexcept(TIsNoexceptCopyConstructible<T>) {
  ASSERT(count >= 0);
  if constexpr (TIsNoexceptCopyConstructible<T>) {
    if constexpr (detail::TIsCopyInitializableWithMemset<T>) {
      if (count)
        ::memset(items, bit_cast<uint8_t>(value), toUnsigned(count));
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
      Destroy(items, i);
      throw;
    }
  }
}

template<typename T, typename U>
inline void Fill(T* items, int count, const U& value) noexcept(TIsNoexceptCopyAssignable<T>) {
  ASSERT(count >= 0);
  if constexpr (TIsNoexceptCopyAssignable<T>) {
    if constexpr (detail::TIsCopyInitializableWithMemset<T>) {
      if (count)
        ::memset(items, bit_cast<uint8_t>(value), toUnsigned(count));
    } else {
      for (int i = 0; i < count; ++i)
        items[i] = value;
    }
  } else {
    for (int i = 0; i < count; ++i)
      items[i] = value;
  }
}

template<typename T>
inline bool Equals(const T* lhs, const T* rhs, int count) noexcept {
  ASSERT(count >= 0);
  if constexpr (TIsTriviallyEqualityComparable<T>) {
    if (count == 0)
      return true;
    return ::memcmp(lhs, rhs, toUnsigned(count) * sizeof(T)) == 0;
  } else {
    for (int i = 0; i < count; ++i) {
      if (lhs[i] != rhs[i])
        return false;
    }
    return true;
  }
}

template<typename T>
inline void Copy(T* dst, const T* src, int count) noexcept(TIsNoexceptCopyAssignable<T>) {
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
inline void CopyNonOverlapping(T* dst, const T* src, int count) noexcept(TIsNoexceptCopyAssignable<T>) {
  ASSERT(count >= 0);
  ASSERT(!AreOverlapping(dst, src, count));
  if constexpr (TIsTriviallyCopyAssignable<T>) {
    if (count != 0)
      ::memcpy(dst, src, toUnsigned(count) * sizeof(T));
  } else {
    for (int i = 0; i < count; ++i)
      dst[i] = src[i];
  }
}

template<typename T, typename U>
inline int indexOfItem(const T* items, int size, const U& item) noexcept {
  ASSERT(size >= 0);
  for (int i = 0; i < size; ++i) {
    if (items[i] == item)
      return i;
  }
  return -1;
}

template<typename T, typename U>
inline int lastIndexOfItem(const T* items, int size, const U& item) noexcept {
  ASSERT(size >= 0);
  for (int i = size - 1; i >= 0; --i) {
    if (items[i] == item)
      return i;
  }
  return -1;
}

template<typename T, typename U>
int Count(const T* items, int size, const U& item) noexcept {
  ASSERT(size >= 0);
  int count = 0;
  while (size > 0) {
    int pos = indexOfItem(items, size, item);
    if (pos < 0)
      break;

    ++count;

    items += pos + 1;
    size -= pos + 1;
  }
  return count;
}

template<typename T, typename TBefore, typename TAfter>
inline int Replace(T* data, int size, const TBefore& before, const TAfter& after) {
  ASSERT(size >= 0);
  int count = 0;
  for (int i = 0; i < size; ++i) {
    if (data[i] == before) {
      data[i] = after;
      ++count;
    }
  }
  return count;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_ARRAYOPS_H_
