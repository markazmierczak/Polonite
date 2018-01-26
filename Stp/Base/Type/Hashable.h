// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_HASHABLE_H_
#define STP_BASE_TYPE_HASHABLE_H_

#include "Base/Type/HashableFwd.h"
#include "Base/Type/Sign.h"
#include "Base/Type/Variable.h"

namespace stp {

BASE_EXPORT HashCode Combine(HashCode first, HashCode second);

BASE_EXPORT HashCode Finalize(HashCode code);

inline HashCode HashMany() { return HashCode::Zero; }

template<typename T>
inline HashCode HashMany(const T& v) { return Hash(v); }

template<typename T, typename... Ts>
inline HashCode HashMany(const T& v, const Ts&... vs) {
  return Combine(HashMany(vs...), Hash(v));
}

struct DefaultHasher {
  template<typename T>
  HashCode operator()(const T& x) const {
    return Finalize(Hash(x));
  }
};

template<typename T, TEnableIf<TIsScalar<T>>*>
inline HashCode Hash(T x) {
  if constexpr (TIsInteger<T>) {
    if constexpr (sizeof(T) <= sizeof(HashCode)) {
      return static_cast<HashCode>(x);
    } else {
      static_assert(sizeof(T) == sizeof(HashCode) * 2, "!");
      TMakeUnsigned<T> y = x;
      return static_cast<HashCode>((y >> 32) ^ y);
    }
  } else if constexpr (TIsEnum<T>) {
    return Hash(ToUnderlying(x));
  } else if constexpr (TIsPointer<T>) {
    // Take lower bits only.
    return static_cast<HashCode>(reinterpret_cast<uintptr_t>(x));
  } else if constexpr (TIsCharacter<T>) {
    return static_cast<HashCode>(char_cast<char32_t>(x));
  } else if constexpr (TIsFloatingPoint<T>) {
    if constexpr (sizeof(T) == 4) {
      // Clear sign bit.
      // The hash collision will be greater but we handle -0.0 == 0.0 case.
      auto y = bit_cast<uint32_t>(x) & ~UINT32_C(0x80000000);
      return static_cast<HashCode>(y);
    } else {
      static_assert(sizeof(T) == 8);
      return Hash(bit_cast<uint64_t>(x) & ~UINT64_C(0x8000000000000000));
    }
  } else if constexpr (TIsBoolean<T>) {
    return static_cast<HashCode>(x);
  } else if constexpr (TIsMemberPointer<T>) {
    static_assert(sizeof(T) >= sizeof(HashCode));
    HashCode rv;
    memcpy(&rv, &x, sizeof(HashCode));
    return rv;
  } else {
    static_assert(TIsNullPointer<T>);
    return HashCode::Zero;
  }
}

template<typename T>
inline HashCode HashContiguous(const T* data, int size) {
  if constexpr (!TIsFloatingPoint<T> && alignof(T) < sizeof(HashCode)) {
    return HashBuffer(data, size * isizeof(T));
  } else {
    HashCode code = HashCode::Zero;
    if (size) {
      code = Hash(data[0]);
      for (int i = 1; i < size; ++i)
        code = Combine(code, Hash(data[i]));
    }
    return code;
  }
}

} // namespace stp

#endif // STP_BASE_TYPE_HASHABLE_H_