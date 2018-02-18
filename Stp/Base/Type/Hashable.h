// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_HASHABLE_H_
#define STP_BASE_TYPE_HASHABLE_H_

#include "Base/Type/Sign.h"
#include "Base/Type/Variable.h"

namespace stp {

BASE_EXPORT HashCode Combine(HashCode first, HashCode second);

BASE_EXPORT HashCode Finalize(HashCode code);

template<typename T, TEnableIf<TIsScalar<T>>* = nullptr>
inline HashCode hash(T x) {
  if constexpr (TIsInteger<T>) {
    if constexpr (sizeof(T) <= sizeof(HashCode)) {
      return static_cast<HashCode>(x);
    } else {
      static_assert(sizeof(T) == sizeof(HashCode) * 2, "!");
      TMakeUnsigned<T> y = x;
      return static_cast<HashCode>((y >> 32) ^ y);
    }
  } else if constexpr (TIsEnum<T>) {
    return hash(toUnderlying(x));
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
      return hash(bit_cast<uint64_t>(x) & ~UINT64_C(0x8000000000000000));
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

namespace detail {

template<typename T>
using THashableConcept = decltype(hash(declval<const T&>()));

} // namespace detail

template<typename T>
constexpr bool TIsHashable = TsAreSame<HashCode, TDetect<detail::THashableConcept, T>>;

inline HashCode HashMany() { return HashCode::Zero; }

template<typename T>
inline HashCode HashMany(const T& v) { return hash(v); }

template<typename T, typename... Ts>
inline HashCode HashMany(const T& v, const Ts&... vs) {
  return Combine(HashMany(vs...), hash(v));
}

struct DefaultHasher {
  template<typename T>
  HashCode operator()(const T& x) const {
    return Finalize(hash(x));
  }
};

} // namespace stp

#endif // STP_BASE_TYPE_HASHABLE_H_
