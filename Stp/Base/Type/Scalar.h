// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_SCALAR_H_
#define STP_BASE_TYPE_SCALAR_H_

#include "Base/Type/CVRef.h"

namespace stp {

template<typename T>
constexpr bool TIsVoid = TsAreSame<void, TRemoveCV<T>>;

template<typename T>
constexpr bool TIsNullPointer = TsAreSame<nullptr_t, TRemoveCV<T>>;

namespace detail {

// TIsCharacter has no counterpart in C++ Standard Library.
// In std all character types are enclosed by std::is_integral.
// std::is_integral is true for bool as well.
//
// Unlike std we make a clear distinction between boolean, integer and character types.
// The reason is the character types has special meaning - their value depends strictly
// on the context, e.g. char32_t cannot contain a value greater than 0x10FFFF for UTF-32.
// Moreover character types are not usually used in arithmetic operations.
//
template<typename T>
struct TIsCharacterTmpl : TFalse {};

template<> struct TIsCharacterTmpl<char> : TTrue {};
template<> struct TIsCharacterTmpl<char16_t> : TTrue {};
template<> struct TIsCharacterTmpl<char32_t> : TTrue {};
template<> struct TIsCharacterTmpl<wchar_t> : TTrue {};

template<typename T>
struct TIsIntegerTmpl : TFalse {};

template<> struct TIsIntegerTmpl<signed char> : TTrue {};
template<> struct TIsIntegerTmpl<unsigned char> : TTrue {};
template<> struct TIsIntegerTmpl<short> : TTrue {};
template<> struct TIsIntegerTmpl<unsigned short> : TTrue {};
template<> struct TIsIntegerTmpl<int> : TTrue {};
template<> struct TIsIntegerTmpl<unsigned int> : TTrue {};
template<> struct TIsIntegerTmpl<long> : TTrue {};
template<> struct TIsIntegerTmpl<unsigned long> : TTrue {};
template<> struct TIsIntegerTmpl<long long> : TTrue {};
template<> struct TIsIntegerTmpl<unsigned long long> : TTrue {};

template<typename T>
struct TIsFloatingPointTmpl : TFalse {};

template<> struct TIsFloatingPointTmpl<float> : TTrue {};
template<> struct TIsFloatingPointTmpl<double> : TTrue {};
template<> struct TIsFloatingPointTmpl<long double> : TTrue {};

template<typename T>
struct TIsPointerTmpl : TFalse {};

template<typename T>
struct TIsPointerTmpl<T*> : TTrue {};

struct TIsFunctionDummyType {};
template<typename T> char TIsFunctionTest(T*);
template<typename T> char TIsFunctionTest(TIsFunctionDummyType);
template<typename T> int TIsFunctionTest(...);
template<typename T> T& TIsFunctionSource(int);
template<typename T> TIsFunctionDummyType TIsFunctionSource(...);

template<typename T,
          bool = TIsClass<T> || TIsUnion<T> || TIsVoid<T> || TIsReference<T> || TIsNullPointer<T>
         >
struct TIsFunctionTmpl : TBoolConstant<sizeof(TIsFunctionTest<T>(TIsFunctionSource<T>(0))) == 1> {};

template<typename T>
struct TIsFunctionTmpl<T, true> : TFalse {};

template<typename T, typename U>
struct TRemovePointerHelper {
  typedef T Type;
};

template<typename T, typename U>
struct TRemovePointerHelper<T, U*> {
  typedef U Type;
};

template<typename T, bool = TIsReferenceable<T> || TIsVoid<T>>
struct TAddPointerHelper {
  typedef T Type;
};

template<typename T>
struct TAddPointerHelper<T, true> {
  typedef TRemoveReference<T>* Type;
};

template<typename T>
struct TIsMemberFunctionPointerTmpl : TFalse {};

template<typename R, typename C>
struct TIsMemberFunctionPointerTmpl<R C::*> : TIsFunctionTmpl<R> {};

template<typename T>
struct TIsMemberPointerTmpl : TFalse {};

template<typename T, typename C>
struct TIsMemberPointerTmpl<T C::*> : TTrue {};

template<typename T, typename... TArgs>
using TFunctionObjectConcept = decltype(declval<const T&>()(declval<TArgs>()...));

template<typename T>
using TNamedEnumConcept = decltype(GetEnumName(declval<T>()));

} // namespace detail

template<typename T>
using TRemovePointer = typename detail::TRemovePointerHelper<T, TRemoveCV<T>>::Type;

template<typename T>
using TAddPointer = typename detail::TAddPointerHelper<T>::Type;

template<typename T>
using TRemoveCVAndPointer = TRemoveCV<TRemovePointer<T>>;

template<typename T>
constexpr bool TIsBoolean = TsAreSame<bool, TRemoveCV<T>>;

template<typename T>
constexpr bool TIsInteger = detail::TIsIntegerTmpl<TRemoveCV<T>>::Value;

template<typename T>
constexpr bool TIsCharacter = detail::TIsCharacterTmpl<TRemoveCV<T>>::Value;

template<typename T>
constexpr bool TIsFloatingPoint = detail::TIsFloatingPointTmpl<TRemoveCV<T>>::Value;

template<typename T>
constexpr bool TIsArithmetic = TIsInteger<T> || TIsFloatingPoint<T>;

template<typename T>
constexpr bool TIsFundamental =
    TIsArithmetic<T> || TIsCharacter<T> || TIsBoolean<T> || TIsVoid<T> || TIsNullPointer<T>;

template<typename T>
constexpr bool TIsPointer = detail::TIsPointerTmpl<TRemoveCV<T>>::Value;

template<typename T>
constexpr bool TIsFunction = detail::TIsFunctionTmpl<T>::Value;

template<typename T, typename TResult, typename... TArgs>
constexpr bool TIsFunctionObject = TsAreSame<TResult, TDetect<detail::TFunctionObjectConcept, T, TArgs...>>;

template<typename T>
constexpr bool TIsMemberPointer = detail::TIsMemberPointerTmpl<TRemoveCV<T>>::Value;

template<typename T>
constexpr bool TIsMemberFunctionPointer =
    detail::TIsMemberFunctionPointerTmpl<TRemoveCV<T>>::Value;

template<typename T>
constexpr bool TIsMemberObjectPointer = TIsMemberPointer<T> && !TIsMemberFunctionPointer<T>;

template<typename T>
constexpr bool TIsScalar =
    TIsFundamental<T> || TIsEnum<T> || TIsPointer<T> || TIsMemberPointer<T>;

template<typename T>
constexpr bool TIsNamedEnum = THasDetected<detail::TNamedEnumConcept, T>;

// Min and Max - Typed minimum and maximum.
// Min(a, b) selects one of following (depending on T type):
// - const reference is returned for non-fundamental types

template<typename T, TEnableIf<TIsFundamental<T>>* = nullptr>
constexpr T Min(T x, T y) {
  return x < y ? x : y;
}

template<typename T, TEnableIf<TIsFundamental<T>>* = nullptr>
constexpr T Max(T x, T y) {
  return x < y ? y : x;
}

// Clamp(x, min, max) - clamps |x| to the [min..max] range.
template<typename T, TEnableIf<TIsFundamental<T>>* = nullptr>
constexpr T Clamp(T x, T min, T max) {
  return Min(Max(x, min), max);
}

// Lerp(a, b, t) - linear interpolation between |a| and |b|.
template<typename T, TEnableIf<TIsArithmetic<T>>* = nullptr>
constexpr T Lerp(T x, T y, double t) {
  return static_cast<T>(x * (1 - t) + y * t);
}

template<typename T>
constexpr T* Coalesce(T* nullable, T* default_not_null) {
  return nullable ? nullable : default_not_null;
}

} // namespace stp

#endif // STP_BASE_TYPE_SCALAR_H_
