// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_BASIC_H_
#define STP_BASE_TYPE_BASIC_H_

#include "Base/Type/Attributes.h"

#include <stddef.h>
#include <stdint.h>

namespace stp {

typedef uint8_t byte_t;
typedef decltype(nullptr) nullptr_t;
enum NoInitTag { NoInit };
enum class HashCode : uint32_t { Zero = 0 };

#define isizeof(x) static_cast<int>(sizeof(x))
#define ialignof(x) static_cast<int>(alignof(x))

template<typename T, int TCount>
constexpr int ArraySizeOf(T (&array)[TCount]) { return TCount; }

#if COMPILER(MSVC)
typedef double max_align_t;
#elif defined(__APPLE__)
typedef long double max_align_t;
#else
typedef struct {
  long long max_align_nonce1 __attribute__((__aligned__(__alignof__(long long))));
  long double max_align_nonce2 __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;
#endif

#if defined(_WIN32)
# define SIZEOF_WCHAR_T 2
#else
# define SIZEOF_WCHAR_T 4
#endif

// Casting chars is tricky since char and wchar_t may be signed.
template<typename T, typename U>
constexpr T char_cast(U x) { return static_cast<T>(x); }

template<typename T>
constexpr T char_cast(char x) { return static_cast<T>(static_cast<unsigned char>(x)); }

template<typename T>
constexpr T char_cast(wchar_t x) {
  #if SIZEOF_WCHAR_T == 2
  return static_cast<T>(static_cast<unsigned short>(x));
  #elif SIZEOF_WCHAR_T == 4
  return static_cast<T>(static_cast<unsigned int>(x));
  #endif
}

template<int TLength, int TAlignment>
struct ALIGNAS(TAlignment) AlignedByteArray {
  byte_t bytes[TLength];
};

template<typename T>
using AlignedStorage = AlignedByteArray<isizeof(T), ialignof(T)>;

template<typename T, T V>
struct TIntegerConstant {
  static constexpr T Value = V;
  typedef T ValueType;
  typedef TIntegerConstant<T, V> Type;

  constexpr operator ValueType() const { return Value; }
};

template<typename T, T V>
constexpr T TIntegerConstant<T, V>::Value;

template<bool Value>
using TBoolConstant = TIntegerConstant<bool, Value>;

typedef TBoolConstant<true> TTrue;
typedef TBoolConstant<false> TFalse;

template<typename...>
using TVoid = void;

struct TUnspecified {
  TUnspecified() = delete;
  ~TUnspecified() = delete;
  TUnspecified(const TUnspecified&) = delete;
  TUnspecified& operator=(const TUnspecified&) = delete;
};

// Returns true if the type has no instance data members.
template<typename T>
constexpr bool TIsEmpty = __is_empty(T);

// Returns true if the type is an enum.
template<typename T>
constexpr bool TIsEnum = __is_enum(T);

// Returns true if a type is an union.
template<typename T>
constexpr bool TIsUnion = __is_union(T);

// Returns true if the type is a class or struct.
template<typename T>
constexpr bool TIsClass = __is_class(T);

// Abbreviation for frequently used combination.
template<typename T>
constexpr bool TIsClassOrUnion = TIsClass<T> || TIsUnion<T>;

// Returns true if the type is a class or union with no constructor or private
// or protected non-static members, no base classes, and no virtual functions.
template<typename T>
constexpr bool TIsPOD = __is_pod(T);

// Returns true if the type is an abstract type.
template<typename T>
constexpr bool TIsAbstract = __is_abstract(T);

template<typename T>
constexpr bool TIsPolymorphic = __is_polymorphic(T);

template<typename T>
constexpr bool TIsFinal = __is_final(T);

template<typename T>
constexpr bool TIsStandardLayout = __is_standard_layout(T);

template<typename T>
constexpr bool TIsTrivial = __is_trivial(T);

template<typename Base, typename Derived>
constexpr bool TIsBaseOf = __is_base_of(Base, Derived);

namespace detail {

template<bool B, typename TIfTrue, typename TIfFalse>
struct TConditionalHelper {
  typedef TIfTrue Type;
};

template<typename TIfTrue, typename TIfFalse>
struct TConditionalHelper<false, TIfTrue, TIfFalse> {
  typedef TIfFalse Type;
};

template<bool, typename T = void>
struct TEnableIfHelper {};

template<typename T>
struct TEnableIfHelper<true, T> {
  typedef T Type;
};

template<typename T1, typename T2, typename... Tx>
struct TsAreSameHelper {
  static constexpr bool Value =
      TsAreSameHelper<T1, T2>::Value &&
      TsAreSameHelper<T2, Tx...>::Value;
};

template<typename T1, typename T2>
struct TsAreSameHelper<T1, T2> : TFalse {};

template<typename T>
struct TsAreSameHelper<T, T> : TTrue {};

template<typename T>
char TIsCompleteHelper(int(*)[sizeof(T)]);

template<typename T>
int TIsCompleteHelper(...);

template<typename T> T&& DeclareHelper(int);
template<typename T> T DeclareHelper(long);

template<typename T>
struct TUnderlyingHelper {
  typedef __underlying_type(T) Type;
};

template<class TDefault, class TAlwaysVoid,
         template<class...> class TOp, class... TArgs>
struct TDetectorHelper {
  static constexpr bool Value = false;
  using Type = TDefault;
};

template<class TDefault, template<class...> class TOp, class... TArgs>
struct TDetectorHelper<TDefault, TVoid<TOp<TArgs...>>, TOp, TArgs...> {
  static constexpr bool Value = true;
  using Type = TOp<TArgs...>;
};

} // namespace detail

template<bool B, typename TIfTrue, typename TIfFalse>
using TConditional = typename detail::TConditionalHelper<B, TIfTrue, TIfFalse>::Type;

template<bool B, typename T = void>
using TEnableIf = typename detail::TEnableIfHelper<B, T>::Type;

template<typename... Tx>
constexpr bool TsAreSame = detail::TsAreSameHelper<Tx...>::Value;

template<typename T>
constexpr bool TIsComplete = sizeof(detail::TIsCompleteHelper<T>(0)) == 1;

template<typename T>
decltype(detail::DeclareHelper<T>(0)) declval() noexcept;

template<typename T>
using TUnderlying = typename detail::TUnderlyingHelper<T>::Type;

template<typename T, TEnableIf<TIsEnum<T>>* = nullptr>
constexpr TUnderlying<T> ToUnderlying(T x) {
  return static_cast<TUnderlying<T>>(x);
}

template<template<class...> class TOp, class... TArgs>
constexpr bool THasDetected = detail::TDetectorHelper<TUnspecified, void, TOp, TArgs...>::Value;

template <template<class...> class TOp, class... TArgs>
using TDetect = typename detail::TDetectorHelper<TUnspecified, void, TOp, TArgs...>::Type;

} // namespace stp

#endif // STP_BASE_TYPE_BASIC_H_