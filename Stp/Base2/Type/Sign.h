// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_SIGN_H_
#define STP_BASE_TYPE_SIGN_H_

#include "Base/Type/Scalar.h"

// Unlike std::is_(un)signed and std::make_(un)signed these equivalents
// does not work for character types, i.e.: wchar_t, char16_t, char32_t.
// char is supported because it is used as 'byte' type.

namespace stp {

namespace detail {

template<typename T, bool = TIsInteger<T>>
struct TIsSignedHelper2 : TBoolConstant<(T(-1) < T(0))> {};

template<typename T>
struct TIsSignedHelper2<T, false> : TTrue {}; // floating point

template<typename T, bool = TIsArithmetic<T>>
struct TIsSignedHelper : TIsSignedHelper2<T> {};

template<typename T>
struct TIsSignedHelper<T, false> : TFalse {};

template<typename T>
struct TMakeIntegerSigned;

template<> struct TMakeIntegerSigned<  signed char     > {typedef signed char Type;};
template<> struct TMakeIntegerSigned<unsigned char     > {typedef signed char Type;};
template<> struct TMakeIntegerSigned<  signed short    > {typedef short     Type;};
template<> struct TMakeIntegerSigned<unsigned short    > {typedef short     Type;};
template<> struct TMakeIntegerSigned<  signed int      > {typedef int       Type;};
template<> struct TMakeIntegerSigned<unsigned int      > {typedef int       Type;};
template<> struct TMakeIntegerSigned<  signed long     > {typedef long      Type;};
template<> struct TMakeIntegerSigned<unsigned long     > {typedef long      Type;};
template<> struct TMakeIntegerSigned<  signed long long> {typedef long long Type;};
template<> struct TMakeIntegerSigned<unsigned long long> {typedef long long Type;};

template<typename T, bool = TIsInteger<T>, bool = TIsEnum<T>>
struct TMakeSignedHelper2 {};

template<typename T>
struct TMakeSignedHelper2<T, true, false> {
  typedef typename TMakeIntegerSigned<T>::Type Type;
};

template<typename T>
struct TMakeSignedHelper2<T, false, true> {
  typedef TUnderlying<T> UnderlyingType;
  typedef typename TMakeIntegerSigned<UnderlyingType>::Type Type;
};

template<typename T>
struct TMakeIntegerUnsigned;

template<> struct TMakeIntegerUnsigned<  signed char     > {typedef unsigned char Type;};
template<> struct TMakeIntegerUnsigned<unsigned char     > {typedef unsigned char Type;};
template<> struct TMakeIntegerUnsigned<  signed short    > {typedef unsigned short     Type;};
template<> struct TMakeIntegerUnsigned<unsigned short    > {typedef unsigned short     Type;};
template<> struct TMakeIntegerUnsigned<  signed int      > {typedef unsigned int       Type;};
template<> struct TMakeIntegerUnsigned<unsigned int      > {typedef unsigned int       Type;};
template<> struct TMakeIntegerUnsigned<  signed long     > {typedef unsigned long      Type;};
template<> struct TMakeIntegerUnsigned<unsigned long     > {typedef unsigned long      Type;};
template<> struct TMakeIntegerUnsigned<  signed long long> {typedef unsigned long long Type;};
template<> struct TMakeIntegerUnsigned<unsigned long long> {typedef unsigned long long Type;};

template<typename T, bool = TIsInteger<T>, bool = TIsEnum<T>>
struct TMakeUnsignedHelper2 {};

template<typename T>
struct TMakeUnsignedHelper2<T, true, false> {
  typedef typename TMakeIntegerUnsigned<T>::Type Type;
};

template<typename T>
struct TMakeUnsignedHelper2<T, false, true> {
  typedef TUnderlying<T> UnderlyingType;
  typedef typename TMakeIntegerUnsigned<UnderlyingType>::Type Type;
};

} // namespace detail

template<typename T>
constexpr bool TIsSigned = detail::TIsSignedHelper<T>::Value;

template<typename T>
constexpr bool TIsUnsigned = TIsArithmetic<T> && !TIsSigned<T>;

template<typename T>
struct TMakeSignedTmpl {
  typedef TApplyCV<T, typename detail::TMakeSignedHelper2<TRemoveCV<T>>::Type> Type;
};

template<typename T>
struct TMakeUnsignedTmpl {
  typedef TApplyCV<T, typename detail::TMakeUnsignedHelper2<TRemoveCV<T>>::Type> Type;
};

template<typename T>
using TMakeSigned = typename TMakeSignedTmpl<T>::Type;

template<typename T>
using TMakeUnsigned = typename TMakeUnsignedTmpl<T>::Type;

// Determines if a numeric value is negative without throwing compiler
// warnings on: unsigned(value) < 0.
template<typename T, TEnableIf<TIsUnsigned<T>>* = nullptr>
constexpr bool IsNegative(T x) {
  return false;
}
template<typename T, TEnableIf<TIsSigned<T>>* = nullptr>
constexpr bool IsNegative(T x) {
  return x < 0;
}

// https://en.wikipedia.org/wiki/Sign_function
template<typename T, TEnableIf<TIsUnsigned<T>>* = nullptr>
constexpr int Signum(T x) {
  return 0 < x;
}

template<typename T, TEnableIf<TIsSigned<T>>* = nullptr>
constexpr int Signum(T x) {
  return (0 < x) - (x < 0);
}

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr TMakeSigned<T> ToSigned(T x) {
  return static_cast<TMakeSigned<T>>(x);
}

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr TMakeUnsigned<T> ToUnsigned(T x) {
  return static_cast<TMakeUnsigned<T>>(x);
}

} // namespace stp

#endif // STP_BASE_TYPE_SIGN_H_
