// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_VARIABLE_H_
#define STP_BASE_TYPE_VARIABLE_H_

#include "Base/Type/Array.h"

namespace stp {

namespace detail {

template<class To, class From>
struct TIsTriviallyAssignableHelper :
    TBoolConstant<__is_trivially_assignable(To, From)> {};

} // namespace detail

template<typename To, typename From>
constexpr bool TIsTriviallyAssignable =
    detail::TIsTriviallyAssignableHelper<To, From>::Value;

template<typename T>
constexpr bool TIsTriviallyCopyable =
    #if COMPILER(GCC) && !COMPILER(CLANG)
    __is_trivially_copyable(T) && !TIsVolatile<T>;
    #else
    __is_trivially_copyable(T);
    #endif

namespace detail {

template<typename T>
class TDecayHelper {
  using U = TRemoveReference<T>;
 public:
  typedef TConditional<TIsArray<U>,
                       TRemoveExtent<U>*,
                       TConditional<TIsFunction<U>, TAddPointer<U>, TRemoveCV<U>>
                      > Type;
};

#if COMPILER(MSVC)

template<typename From, typename To>
struct TIsConvertibleToHelper
    : TIntegerConstant<bool, __is_convertible_to(From, To) && !TIsAbstract<To>> {};
#else

template<typename T>
void TIsConvertibleToTestConvert(T);

template<typename From, typename To, typename = void>
struct TIsConvertibleToTest : TFalse {};

template<typename From, typename To>
struct TIsConvertibleToTest<From, To,
    decltype(TIsConvertibleToTestConvert<To>(declval<From>()))>
    : TTrue {};

template<typename T>
constexpr bool TIsArrayOrFunctionOrVoid =
    TIsArray<T> || TIsFunction<T> || TIsVoid<T>;


template<typename T, bool = TIsArrayOrFunctionOrVoid<TRemoveReference<T>>>
struct TIsConvertibleToCheck {
  static const int Value = 0;
};

template<typename T>
struct TIsConvertibleToCheck<T, false> {
  static const int Value = sizeof(T);
};

template<typename From, typename To,
    bool Disallow = TIsArrayOrFunctionOrVoid<From> || TIsArrayOrFunctionOrVoid<To>>
struct TIsConvertibleToHelper2 : TIntegerConstant<bool, TIsConvertibleToTest<From, To>::Value> {
};

template<typename From, typename To>
struct TIsConvertibleToHelper2<From, To, true>
    : TIntegerConstant<bool, TIsVoid<From> && TIsVoid<To>> {
};

template<typename From, typename To>
struct TIsConvertibleToHelper : TIsConvertibleToHelper2<From, To> {
  static const int Check1 = TIsConvertibleToCheck<From>::Value;
  static const int Check2 = TIsConvertibleToCheck<To>::Value;
};

#endif // COMPILER(MSVC)

template<typename T, typename U>
class TIsAssignableHelper {
  template<typename T1, typename U1, typename = decltype(declval<T1>() = declval<U1>())>
  static TTrue Test(int);

  template<typename, typename>
  static TFalse Test(...);

 public:
  typedef decltype(Test<T, U>(0)) Type;
};

template<typename T, bool = TIsReferenceable<T>>
struct TIsMoveAssignableHelper;

template<typename T>
struct TIsMoveAssignableHelper<T, false> : TFalse {};

template<typename T>
struct TIsMoveAssignableHelper<T, true> : TIsAssignableHelper<T&, T&&>::Type {};

} // namespace detail

template<typename T>
using TDecay = typename detail::TDecayHelper<T>::Type;

template<typename TFrom, typename TTo>
constexpr bool TIsConvertibleTo = detail::TIsConvertibleToHelper<TFrom, TTo>::Value;

template<typename TTo, typename TFrom>
constexpr bool TIsAssignable = detail::TIsAssignableHelper<TTo, TFrom>::Type::Value;

template<typename T>
constexpr bool TIsCopyAssignable =
    TIsAssignable<TAddLvalueReference<T>, TAddLvalueReference<const T>>;

template<typename T>
constexpr bool TIsMoveAssignable = detail::TIsMoveAssignableHelper<T>::Value;

template<typename T>
constexpr bool TIsTriviallyCopyAssignable =
    TIsCopyAssignable<T> && TIsTriviallyAssignable<T&, const T&>;

template<typename T>
constexpr bool TIsTriviallyMoveAssignable =
    TIsMoveAssignable<T> && TIsTriviallyAssignable<T&, T&&>;

template<typename T, typename... TArgs>
constexpr bool TIsTriviallyConstructible = __is_trivially_constructible(T, TArgs...);

template<typename T>
constexpr bool TIsTriviallyDefaultConstructible = TIsTriviallyConstructible<T>;

template<typename T>
constexpr bool TIsTriviallyCopyConstructible =
    TIsTriviallyConstructible<T, TAddLvalueReference<const T>>;

template<typename T>
constexpr bool TIsTriviallyMoveConstructible =
    TIsTriviallyConstructible<T, TAddRvalueReference<T>>;

namespace detail {

// TIsDestructible
//
//  TIsReference -> TTrue
//  TIsFunction -> TFalse
//  TIsVoid -> TFalse
//  an array of unknown bound -> TFalse
//  Otherwise -> "declval<U&>().~U()" is well-formed where U is TRemoveAllExtents<T>

template<typename>
struct TIsDestructibleApply { typedef int Type; };

template<typename T>
struct TIsDestructorWellformed {
  template<typename T1>
  static char Test(typename TIsDestructibleApply<decltype(declval<T1&>().~T1())>::Type);

  template<typename T1>
  static int Test (...);

  static constexpr bool Value = sizeof(Test<T>(12)) == sizeof(char);
};

template<typename T, bool>
struct TIsDestructibleImpl;

template<typename T>
struct TIsDestructibleImpl<T, false>
    : TBoolConstant<TIsDestructorWellformed<TRemoveAllExtents<T>>::Value> {};

template<typename T>
struct TIsDestructibleImpl<T, true> : TTrue {};

template<typename T, bool>
struct TIsDestructibleFalse;

template<typename T>
struct TIsDestructibleFalse<T, false> : TIsDestructibleImpl<T, TIsReference<T>> {};

template<typename T>
struct TIsDestructibleFalse<T, true> : TFalse {};

template<typename T>
struct TIsDestructibleHelper : TIsDestructibleFalse<T, TIsFunction<T>> {};

template<typename T>
struct TIsDestructibleHelper<T[]> : TFalse {};

template<>
struct TIsDestructibleHelper<void> : TFalse {};

} // namespace detail

template<typename T>
constexpr bool TIsDestructible = detail::TIsDestructibleHelper<T>::Value;

template<typename T>
constexpr bool TIsTriviallyDestructible = __has_trivial_destructor(T) && TIsDestructible<T>;

#if !COMPILER(MSVC)

namespace detail {

template<typename T, typename... TArgs>
struct TIsConstructibleHelper;

template<typename TTo, typename TFrom>
struct TIsInvalidBaseToDerivedCast {
  static_assert(TIsReference<TTo>, "!");
  using RawFrom = TRemoveCVRef<TFrom>;
  using RawTo = TRemoveCVRef<TTo>;

  static constexpr bool Value =
      !TsAreSame<RawFrom, RawTo> &&
      TIsBaseOf<RawFrom, RawTo> &&
      !TIsConstructibleHelper<RawTo, TFrom>::Vvalue;
};

template<typename TTo, typename TFrom>
struct TIsInvalidLvalueToRvalueCast : TFalse {
  static_assert(TIsReference<TTo>, "!");
};

template<typename TToRef, typename TFromRef>
struct TIsInvalidLvalueToRvalueCast<TToRef&&, TFromRef&> {
  using RawFrom = TRemoveCVRef<TFromRef>;
  using RawTo = TRemoveCVRef<TToRef>;

  static constexpr bool Value =
      !TIsFunction<RawTo> &&
      (TsAreSame<RawFrom, RawTo> || TIsBaseOf<RawTo, RawFrom>);
};

struct TIsConstructibleTests {
  template<typename To>
  static void Eat(To);

  // This overload is needed to work around a Clang bug that disallows
  // static_cast<T&&>(e) for non-reference-compatible types.
  // Example: static_cast<int&&>(declval<double>());
  // NOTE: The static_cast implementation below is required to support
  //  typenames with explicit conversion operators.
  template<typename To, typename From,
            typename = decltype(Eat<To>(declval<From>()))>
  static TTrue TestCast(int);

  template<typename To, typename From,
            typename = decltype(static_cast<To>(declval<From>()))>
  static TBoolConstant<
      !TIsInvalidBaseToDerivedCast<To, From>::Value &&
      !TIsInvalidLvalueToRvalueCast<To, From>::Value
  > TestCast(long);

  template<typename, typename>
  static TFalse TestCast(...);

  template<typename T, typename... Args, typename = decltype(T(declval<Args>()...))>
  static TTrue TestNAry(int);
  template<typename T, typename...>
  static TFalse TestNAry(...);

  template<typename T, typename A0, typename = decltype(::new T(declval<A0>()))>
  static TIsDestructibleHelper<T> TestUnary(int);
  template<typename, typename>
  static TFalse TestUnary(...);
};

template<typename T, bool = TIsVoid<T>>
struct TIsDefaultConstructibleHelper : decltype(TIsConstructibleTests::TestNAry<T>(0)) {};

template<typename _Tp>
struct TIsDefaultConstructibleHelper<_Tp, true> : TFalse {};

template<typename _Tp>
struct TIsDefaultConstructibleHelper<_Tp[], false> : TFalse {};

template<typename T, int N>
struct TIsDefaultConstructibleHelper<T[N], false>
    : TIsDefaultConstructibleHelper<TRemoveAllExtents<T>>  {};

template<typename T, typename... TArgs>
struct TIsConstructibleHelper {
  static_assert(sizeof...(TArgs) > 1, "!");
  typedef decltype(TIsConstructibleTests::TestNAry<T, TArgs...>(0)) Type;
};

template<typename T>
struct TIsConstructibleHelper<T> : TIsDefaultConstructibleHelper<T> {};

template<typename Tp, typename A0>
struct TIsConstructibleHelper<Tp, A0>
    : decltype(TIsConstructibleTests::TestUnary<Tp, A0>(0)) {
};

template<typename T, typename A0>
struct TIsConstructibleHelper<T&, A0>
    : decltype(TIsConstructibleTests::TestCast<T&, A0>(0)) {
};

template<typename T, typename A0>
struct TIsConstructibleHelper<T&&, A0>
    : decltype(TIsConstructibleTests::TestCast<T&&, A0>(0)) {
};

} // namespace detail

template<typename T, typename... TArgs>
constexpr bool TIsConstructible = detail::TIsConstructibleHelper<T, TArgs...>::Value;

#else

template<typename T, typename... Args>
constexpr bool TIsConstructible = __is_constructible(T, Args...);

#endif // COMPILER(MSVC)

template<typename T>
constexpr bool TIsDefaultConstructible = TIsConstructible<T>;

template<typename T>
constexpr bool TIsCopyConstructible = TIsConstructible<T, TAddLvalueReference<const T>>;

template<typename T>
constexpr bool TIsMoveConstructible = TIsConstructible<T, TAddRvalueReference<T>>;

namespace detail {

template<bool TIsConstructible, bool TIsReference, typename T, typename... TArgs>
struct TIsNoexceptConstructibleHelper2;

template<typename T, typename... TArgs>
struct TIsNoexceptConstructibleHelper2<true, false, T, TArgs...>
    : public TBoolConstant<noexcept(T(declval<TArgs>()...))> {
};

template<typename T>
void checkImplicitConversionTo(T) noexcept {}

template<typename T, typename TArg>
struct TIsNoexceptConstructibleHelper2<true, true, T, TArg>
    : public TBoolConstant<noexcept(checkImplicitConversionTo<T>(declval<TArg>()))> {};

template<typename T, bool TIsReference, typename... TArgs>
struct TIsNoexceptConstructibleHelper2<false, TIsReference, T, TArgs...> : public TFalse {};

template<typename T, typename... TArgs>
struct TIsNoexceptConstructibleHelper : TIsNoexceptConstructibleHelper2<
    TIsConstructible<T, TArgs...>, TIsReference<T>, T, TArgs...> {};

template<typename T, int N>
struct TIsNoexceptConstructibleHelper<T[N]> : TIsNoexceptConstructibleHelper2<
    TIsConstructible<T>, TIsReference<T>, T> {};

template<bool TIsAssignable, typename T, typename TArg>
struct TIsNoexceptAssignableHelper;

template<typename T, typename TArg>
struct TIsNoexceptAssignableHelper<false, T, TArg> : public TFalse {};

template<typename T, typename TArg>
struct TIsNoexceptAssignableHelper<true, T, TArg>
    : public TBoolConstant<noexcept(declval<T>() = declval<TArg>())> {};

template<bool, typename T>
struct TIsNoexceptDestructibleHelper2;

template<typename T>
struct TIsNoexceptDestructibleHelper2<false, T> : public TFalse {};

template<typename T>
struct TIsNoexceptDestructibleHelper2<true, T>
    : public TBoolConstant<noexcept(declval<T>().~T())> {};

template<typename T>
struct TIsNoexceptDestructibleHelper
    : public TIsNoexceptDestructibleHelper2<TIsDestructible<T>, T> {};

template<typename T, int N>
struct TIsNoexceptDestructibleHelper<T[N]>
    : public TIsNoexceptDestructibleHelper<T> {};

template<typename T>
struct TIsNoexceptDestructibleHelper<T&> : public TTrue {};

template<typename T>
struct TIsNoexceptDestructibleHelper<T&&> : public TTrue {};

} // namespace detail

template<typename T, typename... TArgs>
constexpr bool TIsNoexceptConstructible =
    detail::TIsNoexceptConstructibleHelper<T, TArgs...>::Value;

template<typename T>
constexpr bool TIsNoexceptDefaultConstructible = TIsNoexceptConstructible<T>;

template<typename T>
constexpr bool TIsNoexceptCopyConstructible =
    TIsNoexceptConstructible<T, TAddLvalueReference<const T>>;

template<typename T>
constexpr bool TIsNoexceptMoveConstructible =
    TIsNoexceptConstructible<T, TAddRvalueReference<T>>;

template<typename T, typename TArg>
constexpr bool TIsNoexceptAssignable =
    detail::TIsNoexceptAssignableHelper<TIsAssignable<T, TArg>, T, TArg>::Value;

template<typename T>
constexpr bool TIsNoexceptCopyAssignable =
    TIsNoexceptAssignable<T, TAddLvalueReference<const T>>;

template<typename T>
constexpr bool TIsNoexceptMoveAssignable =
    TIsNoexceptAssignable<T, TAddRvalueReference<T>>;

template<typename T>
constexpr bool TIsNoexceptDestructible =
    detail::TIsNoexceptDestructibleHelper<T>::Value;

template<typename T, typename = void>
struct TIsZeroConstructibleTmpl : TFalse {};

template<typename T>
struct TIsZeroConstructibleTmpl<T, TEnableIf<TIsFundamental<T> || TIsTriviallyConstructible<T>>> : TTrue {};

template<typename T>
constexpr bool TIsZeroConstructible = TIsZeroConstructibleTmpl<T>::Value;

template<class TDst, class TSrc>
inline TDst bitCast(const TSrc& source) noexcept {
  static_assert(sizeof(TDst) == sizeof(TSrc),
                "bitCast requires source and destination to be the same size");
  static_assert(TIsTriviallyCopyable<TDst>,
                "non-trivially-copyable bitCast is undefined");
  static_assert(TIsTriviallyCopyable<TSrc>,
                "non-trivially-copyable bitCast is undefined");

  union {
    TDst d;
    TSrc s;
  };
  s = source;
  return d;
}

namespace detail {

template<typename T>
constexpr bool TIsGenericSwappable = TIsMoveAssignable<T> && TIsMoveConstructible<T>;

template<typename T>
struct TIsSwappableTmpl;

} // namespace detail

template<typename T, TEnableIf<detail::TIsGenericSwappable<T>>* = nullptr>
constexpr void swap(T& x, T& y) noexcept {
  T t(move(x));
  x = move(y);
  y = move(t);
}

template<typename T, int N, TEnableIf<detail::TIsSwappableTmpl<T>::Value>* = nullptr>
constexpr void swap(T (&a)[N], T (&b)[N]) noexcept {
  for (int i = 0; i < N; ++i)
    swap(a[i], b[i]);
}

template<typename T, typename U = T>
constexpr T exchange(T& obj, U&& new_val) noexcept {
  T val = forward<U>(new_val);
  swap(val, obj);
  return val;
}

namespace detail {

template<typename T>
using TSwappableConcept = decltype(swap(declval<T&>(), declval<T&>()));

template<typename T>
struct TIsSwappableTmpl : TBoolConstant<THasDetected<detail::TSwappableConcept, T>> {};

} // namespace detail

template<typename T>
constexpr bool TIsSwappable = detail::TIsSwappableTmpl<T>::Value;

template<typename T, typename = void>
struct TIsTriviallyRelocatableTmpl : TBoolConstant<TIsTriviallyCopyable<T>> {};

template<typename T>
constexpr bool TIsTriviallyRelocatable = TIsTriviallyRelocatableTmpl<T>::Value;

template<typename T>
inline void destroyObject(T& item) noexcept {
  item.~T();
}

template<typename T>
inline void relocateObject(T* target, T& source) {
  if constexpr (TIsTriviallyRelocatable<T>) {
    memcpy(target, &source, sizeof(T));
  } else {
    new (target) T(move(source));
    detroyObject(&source);
  }
}

template<typename T, typename = void>
struct TIsTriviallyEqualityComparableTmpl :
    TBoolConstant<TIsScalar<T> && !TIsFloatingPoint<T>> {};

template<typename T>
constexpr bool TIsTriviallyEqualityComparable = TIsTriviallyEqualityComparableTmpl<T>::Value;

namespace detail {

template<typename T, typename U>
using TEqualityComparableConcept = decltype(declval<const T&>() == declval<const U&>());

} // namespace detail

template<typename T, typename U>
constexpr bool TIsEqualityComparableWith =
    TsAreSame<bool, TDetect<detail::TEqualityComparableConcept, T, U>>;

template<typename T>
constexpr bool TIsEqualityComparable = TIsEqualityComparableWith<T, T>;

struct DefaultEqualityComparer {
  template<typename T, typename U>
  constexpr bool operator()(const T& x, const U& y) const noexcept {
    return x == y;
  }
};

BASE_EXPORT HashCode hashBuffer(const void* data, int size) noexcept;

class TextWriter;

} // namespace stp

#endif // STP_BASE_TYPE_VARIABLE_H_
