// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_SAFE_H_
#define STP_BASE_MATH_SAFE_H_

#include "Base/Math/OverflowMath.h"
#include "Base/Math/SafeConversions.h"

namespace stp {

template<typename T>
class Safe {
 public:
  static_assert(TIsArithmetic<T>, "!");

  constexpr explicit operator bool() const { return !!value_; }
  constexpr bool operator!() const { return !value_; }

  ALWAYS_INLINE constexpr T get() const { return value_; }

  constexpr const T* operator&() const { return &value_; }
  constexpr T* operator&() { return &value_; }

  constexpr Safe& operator++();
  constexpr Safe& operator--();

  constexpr Safe operator++(int) { Safe r = *this; operator++(); return r; }
  constexpr Safe operator--(int) { Safe r = *this; operator--(); return r; }

  template<typename U>
  constexpr operator U() const { return assertedCast<U>(value_); }

  template<typename U>
  constexpr operator Safe<U>() const { return Safe<U>(value_); }

  template<typename U, TEnableIf<TIsArithmetic<U>>* = nullptr>
  constexpr Safe(U other) : value_(assertedCast<T>(other)) {}

 private:
  T value_;
};

template<typename T, TEnableIf<TIsArithmetic<T>>* = nullptr>
constexpr Safe<T> makeSafe(T x) { return Safe<T>(x); }

template<typename T>
constexpr Safe<T> makeSafe(Safe<T> x) { return x; }

namespace safe {

template<typename TLhs, typename TRhs, typename TEnabler>
struct TSafePromotion;

template<typename TLhs, typename TRhs>
struct TSafePromotion<TLhs, TRhs, TEnableIf<TIsSigned<TLhs> == TIsSigned<TRhs>>> {
  typedef decltype(declval<TLhs>() + declval<TRhs>()) Type;
};

template<typename T>
struct TNegOpPromote;

template<> struct TNegOpPromote<  signed char> { typedef int Type; };
template<> struct TNegOpPromote<unsigned char> { typedef int Type; };
template<> struct TNegOpPromote<  signed short> { typedef int Type; };
template<> struct TNegOpPromote<unsigned short> { typedef int Type; };
template<> struct TNegOpPromote<  signed int> { typedef int Type; };
template<> struct TNegOpPromote<unsigned int> { typedef int64_t Type; };
template<> struct TNegOpPromote<  signed long> { typedef long Type; };
template<> struct TNegOpPromote<unsigned long> { typedef int64_t Type; };
template<> struct TNegOpPromote<  signed long long> { typedef long long Type; };


template<typename T, bool TIsSigned = TIsSigned<T>, bool TIsMoreThan32Bit = (sizeof(T) > 4)>
struct TBinaryOpPromoteMixedInts;

template<typename T, bool TIsMoreThan32Bit>
struct TBinaryOpPromoteMixedInts<T, true, TIsMoreThan32Bit> {
  typedef T Type;
};

template<typename T>
struct TBinaryOpPromoteMixedInts<T, false, false> {
  typedef int64_t Type;
};

template<typename T>
struct TBinaryOpPromoteMixedInts<T, false, true> {
  typedef TUnspecified Type;
};

template<typename R, bool TIsFloatingPoint, bool TMixedSign>
struct TBinaryOpPromote2;

template<typename R, bool TMixedSign>
struct TBinaryOpPromote2<R, true, TMixedSign> {
  typedef R Type;
};

template<typename R>
struct TBinaryOpPromote2<R, false, false> {
  typedef R Type;
};

template<typename R>
struct TBinaryOpPromote2<R, false, true> {
  typedef typename TBinaryOpPromoteMixedInts<R>::Type Type;
};

template<typename T, typename U>
struct TBinaryOpPromote {
  typedef decltype(declval<T>() + declval<U>()) BuiltinPromotedType;
  typedef typename TBinaryOpPromote2<
      BuiltinPromotedType,
      TIsFloatingPoint<BuiltinPromotedType>,
      (TIsSigned<T> != TIsSigned<U>)>::Type Type;
};

template<typename T>
constexpr T extract(Safe<T> x) { return x.get(); }

template<typename T>
constexpr T extract(T x) { return x; }

template<typename T>
struct TIsSafeHelper : TFalse {};

template<typename T>
struct TIsSafeHelper<Safe<T>> : TTrue {};

template<typename T>
constexpr bool TIsSafe = TIsSafeHelper<T>::Value;

template<typename T, typename U>
constexpr bool TAreValidBinaryArguments =
    (TIsSafe<T> || TIsSafe<U>) &&
    (TIsSafe<T> || TIsArithmetic<T>) && (TIsSafe<U> || TIsArithmetic<U>);

template<typename T>
constexpr auto Not(T x) { return ~x; }

template<typename T, typename U>
constexpr auto And(T x, U y) { return x & y; }

template<typename T, typename U>
constexpr auto Or (T x, U y) { return x | y; }

template<typename T, typename U>
constexpr auto Xor(T x, U y) { return x ^ y; }

template<typename T>
constexpr auto Pos(T x) { return +x; }

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr auto Neg(T x) {
  using PromotedType = typename TNegOpPromote<T>::Type;
  auto rx = static_cast<PromotedType>(x);
  ASSERT(rx != Limits<PromotedType>::Min);
  return -rx;
}

template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
constexpr auto Neg(T x) {
  return -x;
}

template<typename T>
constexpr bool TNeedsCheckForAbs = TIsInteger<T> && TIsSigned<T>;

template<typename T, TEnableIf<TNeedsCheckForAbs<T>>* = nullptr>
constexpr auto mathAbs(T x) {
  ASSERT(x != Limits<T>::Min);
  return stp::mathAbs(x);
}

template<typename T, TEnableIf<!TNeedsCheckForAbs<T>>* = nullptr>
constexpr auto mathAbs(T x) {
  return stp::mathAbs(x);
}

template<typename T, typename TEnabler = void>
struct ArithmeticOpOverflow {
  static bool add(T x, T y) {
    T dummy;
    return overflowAdd(x, y, &dummy);
  }
  static bool sub(T x, T y) {
    T dummy;
    return overflowSub(x, y, &dummy);
  }
  static bool mul(T x, T y) {
    T dummy;
    return overflowMul(x, y, &dummy);
  }
  static bool div(T x, T y) {
    T dummy;
    return overflowDiv(x, y, &dummy);
  }
  static bool mod(T x, T y) {
    ASSERT(y != 0);
    return false;
  }

  template<typename TShift>
  static bool shiftLeft(T x, TShift shift) {
    T dummy;
    return overflowShiftLeft(x, shift, &dummy);
  }
};

template<typename T>
struct ArithmeticOpOverflow<T, TEnableIf<TIsFloatingPoint<T>>> {
  static bool add(T x, T y) { return false; }
  static bool sub(T x, T y) { return false; }
  static bool mul(T x, T y) { return false; }
  static bool div(T x, T y) { return false; }
  static bool mod(T x, T y) { return false; }
};

template<typename T, typename U>
constexpr auto add(T x, U y) {
  using PromotedType = typename TBinaryOpPromote<T, U>::Type;
  ASSERT(!ArithmeticOpOverflow<PromotedType>::add(x, y));
  return static_cast<PromotedType>(x) + static_cast<PromotedType>(y);
}

template<typename T, typename U>
constexpr auto sub(T x, U y) {
  using PromotedType = typename TBinaryOpPromote<T, U>::Type;
  ASSERT(!ArithmeticOpOverflow<PromotedType>::sub(x, y));
  return static_cast<PromotedType>(x) - static_cast<PromotedType>(y);
}

template<typename T, typename U>
constexpr auto mul(T x, U y) {
  using PromotedType = typename TBinaryOpPromote<T, U>::Type;
  ASSERT(!ArithmeticOpOverflow<PromotedType>::mul(x, y));
  return static_cast<PromotedType>(x) * static_cast<PromotedType>(y);
}

template<typename T, typename U>
constexpr auto div(T x, U y) {
  using PromotedType = typename TBinaryOpPromote<T, U>::Type;
  ASSERT(!ArithmeticOpOverflow<PromotedType>::div(x, y));
  return static_cast<PromotedType>(x) / static_cast<PromotedType>(y);
}

template<typename T, typename U>
constexpr auto mod(T x, U y) {
  using PromotedType = typename TBinaryOpPromote<T, U>::Type;
  ASSERT(!ArithmeticOpOverflow<PromotedType>::mod(x, y));
  return static_cast<PromotedType>(x) % static_cast<PromotedType>(y);
}

template<typename T, typename U>
constexpr auto shiftLeft(T x, U y) {
  typedef decltype(x << y) ResultType;
  ASSERT(!ArithmeticOpOverflow<ResultType>::shiftLeft(x, y));
  return makeSafe(static_cast<ResultType>(toUnsigned(x) << y));
}

template<typename T, typename U>
constexpr auto shiftRight(T x, U y) {
  ASSERT(!isNegative(y) && y < static_cast<U>(8 * sizeof(x)));
  return makeSafe(x >> y);
}

enum class ComparisonMethod {
  NoCast,
  CastToInt,
  CastToInt64,
  UnsignedLhs,
  UnsignedRhs,
};

template<typename TLhs, typename TRhs,
         bool TMixedSign = (TIsSigned<TLhs> != TIsSigned<TRhs>)>
struct TSelectComparisonMethodForInts {
  static constexpr ComparisonMethod Value = ComparisonMethod::NoCast;
};

template<typename TLhs, typename TRhs>
struct TSelectComparisonMethodForInts<TLhs, TRhs, true> {
  static constexpr ComparisonMethod Value =
      ((TIsSigned<TLhs> && sizeof(TLhs) < 8 && sizeof(TRhs) < 4) ||
       (TIsSigned<TRhs> && sizeof(TRhs) < 8 && sizeof(TLhs) < 4)) ? ComparisonMethod::CastToInt :
      ((TIsSigned<TLhs> && sizeof(TRhs) < 8) ||
       (TIsSigned<TRhs> && sizeof(TLhs) < 8)) ? ComparisonMethod::CastToInt64 :
      TIsUnsigned<TLhs> ? ComparisonMethod::UnsignedLhs : ComparisonMethod::UnsignedRhs;
};

template<typename TLhs, typename TRhs,
         bool TEitherFloatingPoint = TIsFloatingPoint<TLhs> || TIsFloatingPoint<TRhs>>
struct TSelectComparisonMethod {
  static_assert(sizeof(TLhs) <= 8 && sizeof(TRhs) <= 8, "!");
  static constexpr ComparisonMethod Value = TSelectComparisonMethodForInts<TLhs, TRhs>::Value;
};

template<typename TLhs, typename TRhs>
struct TSelectComparisonMethod<TLhs, TRhs, true> {
  static constexpr ComparisonMethod Value = ComparisonMethod::NoCast;
};

template<typename TLhs, typename TRhs,
         ComparisonMethod TMethod = TSelectComparisonMethod<TLhs, TRhs>::Value>
struct CompareOp;

template<typename TLhs, typename TRhs>
struct CompareOp<TLhs, TRhs, ComparisonMethod::NoCast> {
  static bool Eq(TLhs x, TRhs y) { return x == y; }
  static bool Ne(TLhs x, TRhs y) { return x != y; }
  static bool Gt(TLhs x, TRhs y) { return x >  y; }
  static bool Lt(TLhs x, TRhs y) { return x <  y; }
  static bool Ge(TLhs x, TRhs y) { return x >= y; }
  static bool Le(TLhs x, TRhs y) { return x <= y; }
};

template<typename TLhs, typename TRhs>
struct CompareOp<TLhs, TRhs, ComparisonMethod::CastToInt> {
  static bool Eq(TLhs x, TRhs y) {
    return static_cast<int>(x) == static_cast<int>(y);
  }
  static bool Ne(TLhs x, TRhs y) {
    return static_cast<int>(x) != static_cast<int>(y);
  }
  static bool Gt(TLhs x, TRhs y) {
    return static_cast<int>(x) >  static_cast<int>(y);
  }
  static bool Lt(TLhs x, TRhs y) {
    return static_cast<int>(x) <  static_cast<int>(y);
  }
  static bool Ge(TLhs x, TRhs y) {
    return static_cast<int>(x) >= static_cast<int>(y);
  }
  static bool Le(TLhs x, TRhs y) {
    return static_cast<int>(x) <= static_cast<int>(y);
  }
};

template<typename TLhs, typename TRhs>
struct CompareOp<TLhs, TRhs, ComparisonMethod::CastToInt64> {
  static bool Eq(TLhs x, TRhs y) {
    return static_cast<int64_t>(x) == static_cast<int64_t>(y);
  }
  static bool Ne(TLhs x, TRhs y) {
    return static_cast<int64_t>(x) != static_cast<int64_t>(y);
  }
  static bool Gt(TLhs x, TRhs y) {
    return static_cast<int64_t>(x) >  static_cast<int64_t>(y);
  }
  static bool Lt(TLhs x, TRhs y) {
    return static_cast<int64_t>(x) <  static_cast<int64_t>(y);
  }
  static bool Ge(TLhs x, TRhs y) {
    return static_cast<int64_t>(x) >= static_cast<int64_t>(y);
  }
  static bool Le(TLhs x, TRhs y) {
    return static_cast<int64_t>(x) <= static_cast<int64_t>(y);
  }
};

template<typename TLhs, typename TRhs>
struct CompareOp<TLhs, TRhs, ComparisonMethod::UnsignedLhs> {
  static bool Eq(TLhs x, TRhs y) {
    if (y < 0)
      return false;
    return x == toUnsigned(y);
  }
  static bool Ne(TLhs x, TRhs y) { return !Eq(x, y); }

  static bool Gt(TLhs x, TRhs y) {
    if (y < 0)
      return true;
    return x > toUnsigned(y);
  }
  static bool Lt(TLhs x, TRhs y) {
    if (y < 0)
      return false;
    return x < toUnsigned(y);
  }
  static bool Le(TLhs x, TRhs y) { return !Gt(x, y); }
  static bool Ge(TLhs x, TRhs y) { return !Lt(x, y); }
};

template<typename TLhs, typename TRhs>
struct CompareOp<TLhs, TRhs, ComparisonMethod::UnsignedRhs> {
  static bool Eq(TLhs x, TRhs y) {
    if (x < 0)
      return false;
    return toUnsigned(x) == y;
  }
  static bool Ne(TLhs x, TRhs y) { return !Eq(x, y); }

  static bool Gt(TLhs x, TRhs y) {
    if (x < 0)
      return false;
    return toUnsigned(x) > y;
  }
  static bool Lt(TLhs x, TRhs y) {
    if (x < 0)
      return true;
    return toUnsigned(x) < y;
  }
  static bool Le(TLhs x, TRhs y) { return !Gt(x, y); }
  static bool Ge(TLhs x, TRhs y) { return !Lt(x, y); }
};

template<typename T, typename U>
constexpr bool CompareEq(T x, U y) { return CompareOp<T, U>::Eq(x, y); }

template<typename T, typename U>
constexpr bool CompareNe(T x, U y) { return CompareOp<T, U>::Ne(x, y); }

template<typename T, typename U>
constexpr bool CompareGt(T x, U y) { return CompareOp<T, U>::Gt(x, y); }

template<typename T, typename U>
constexpr bool CompareLt(T x, U y) { return CompareOp<T, U>::Lt(x, y); }

template<typename T, typename U>
constexpr bool CompareGe(T x, U y) { return CompareOp<T, U>::Ge(x, y); }

template<typename T, typename U>
constexpr bool CompareLe(T x, U y) { return CompareOp<T, U>::Le(x, y); }

} // namespace safe

template<typename T>
struct TMakeSignedTmpl<Safe<T>> {
  typedef Safe<TMakeSigned<T>> Type;
};

template<typename T>
struct TMakeUnsignedTmpl<Safe<T>> {
  typedef Safe<TMakeUnsigned<T>> Type;
};

#define SAFE_UNARY_OPERATOR(OP, NAME) \
  template<typename T> \
  constexpr auto operator OP(Safe<T> x) { \
    return makeSafe(safe::NAME(x.get())); \
  }

SAFE_UNARY_OPERATOR(~, Not)
SAFE_UNARY_OPERATOR(+, Pos)
SAFE_UNARY_OPERATOR(-, Neg)

#undef SAFE_UNARY_OPERATOR

#define SAFE_BINARY_OPERATOR(OP, NAME) \
  template<typename T, typename U, TEnableIf<safe::TAreValidBinaryArguments<T, U>>* = nullptr> \
  constexpr auto operator OP(T x, U y) { \
    return makeSafe(safe::NAME(safe::extract(x), safe::extract(y))); \
  }

SAFE_BINARY_OPERATOR(&, And)
SAFE_BINARY_OPERATOR(|, Or)
SAFE_BINARY_OPERATOR(^, Xor)
SAFE_BINARY_OPERATOR(+, add)
SAFE_BINARY_OPERATOR(-, sub)
SAFE_BINARY_OPERATOR(*, mul)
SAFE_BINARY_OPERATOR(/, div)
SAFE_BINARY_OPERATOR(%, mod)
SAFE_BINARY_OPERATOR(<<, shiftLeft)
SAFE_BINARY_OPERATOR(>>, shiftRight)

#undef SAFE_BINARY_OPERATOR

#define SAFE_COMPARE_OPERATOR(OP, NAME) \
  template<typename T, typename U, TEnableIf<safe::TAreValidBinaryArguments<T, U>>* = nullptr> \
  constexpr bool operator OP(T x, U y) { \
    return safe::NAME(safe::extract(x), safe::extract(y)); \
  }

SAFE_COMPARE_OPERATOR(==, CompareEq)
SAFE_COMPARE_OPERATOR(!=, CompareNe)
SAFE_COMPARE_OPERATOR(< , CompareLt)
SAFE_COMPARE_OPERATOR(> , CompareGt)
SAFE_COMPARE_OPERATOR(<=, CompareLe)
SAFE_COMPARE_OPERATOR(>=, CompareGe)

#undef SAFE_COMPARE_OPERATOR

#define SAFE_COMPOUND_OPERATOR(OP) \
  template<typename T, typename U, TEnableIf<safe::TAreValidBinaryArguments<T, U>>* = nullptr> \
  constexpr T& operator OP##=(T& x, U y) { \
    x = x OP y; \
    return x; \
  }

SAFE_COMPOUND_OPERATOR(&)
SAFE_COMPOUND_OPERATOR(|)
SAFE_COMPOUND_OPERATOR(^)
SAFE_COMPOUND_OPERATOR(+)
SAFE_COMPOUND_OPERATOR(-)
SAFE_COMPOUND_OPERATOR(*)
SAFE_COMPOUND_OPERATOR(/)
SAFE_COMPOUND_OPERATOR(%)
SAFE_COMPOUND_OPERATOR(<<)
SAFE_COMPOUND_OPERATOR(>>)

#undef SAFE_COMPOUND_OPERATOR

template<typename T>
inline constexpr Safe<T>& Safe<T>::operator++() {
  ASSERT(value_ != Limits<T>::Max);
  ++value_;
  return *this;
}

template<typename T>
inline constexpr Safe<T>& Safe<T>::operator--() {
  ASSERT(value_ != Limits<T>::Min);
  --value_;
  return *this;
}

template<typename T>
constexpr auto mathAbs(Safe<T> x) {
  return makeSafe(safe::mathAbs(x.get()));
}

template<typename T>
constexpr auto mathAbsToUnsigned(Safe<T> x) {
  return makeSafe(mathAbsToUnsigned(x.get()));
}

template<typename T>
constexpr auto toUnsigned(Safe<T> x) {
  static_assert(TIsInteger<T>, "!");
  ASSERT(!isNegative(x.get()));
  return makeSafe(toUnsigned(x.get()));
}
template<typename T>
constexpr Safe<TMakeSigned<T>> toSigned(Safe<T> x) {
  static_assert(TIsInteger<T>, "!");
  ASSERT(x.get() <= static_cast<T>(Limits<TMakeSigned<T>>::Max));
  return makeSafe(toSigned(x.get()));
}

template<typename T>
constexpr bool isNegative(Safe<T> x) {
  return isNegative(x.get());
}

template<typename T>
constexpr int mathSignum(Safe<T> x) {
  return mathSignum(x.get());
}

template<typename TDst, typename TSrc>
constexpr Safe<TDst> assertedCast(Safe<TSrc> x) {
  return makeSafe(assertedCast<TDst>(x.get()));
}

template<typename T, typename U, TEnableIf<safe::TAreValidBinaryArguments<T, U>>* = nullptr>
constexpr auto lerp(T x, U y, double t) {
  return makeSafe(lerp(safe::extract(x), safe::extract(y), t));
}

} // namespace stp

#endif // STP_BASE_MATH_SAFE_H_
