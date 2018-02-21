// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_TUPLE_H_
#define STP_BASE_UTIL_TUPLE_H_

#include "Base/Type/ParameterPack.h"
#include "Base/Type/Variable.h"
#include "Base/Util/IntegerSequence.h"

namespace stp {

namespace detail {

template<int I, typename H, bool = TIsEmpty<H> && !TIsFinal<H>>
class TupleLeaf;

template<int I, typename H, bool>
class TupleLeaf {
  H value;

  template<typename T>
  static constexpr bool CanBindReference() {
    return !TIsReference<H> ||
           (TIsLvalueReference<H> && TIsLvalueReference<T>) ||
           (TIsRvalueReference<H> && !TIsLvalueReference<T>);
  }

  TupleLeaf& operator=(const TupleLeaf&);

 public:
  constexpr TupleLeaf() {
    static_assert(!TIsReference<H>,
                  "Attempted to default construct a reference element in a tuple");
  }

  template<typename T>
  constexpr explicit TupleLeaf(T&& other) : value(stp::forward<T>(other)) {
    static_assert(CanBindReference<T>(),
                  "Attempted to construct a reference element in a tuple with an rvalue");
  }

  TupleLeaf(const TupleLeaf& other) = default;
  TupleLeaf(TupleLeaf&& other) = default;

  template<typename T>
  TupleLeaf& operator=(T&& other) {
    value = stp::forward<T>(other);
    return *this;
  }

  int swapWith(TupleLeaf& other) {
    swap(value, other.value);
    return 0;
  }

  constexpr       H& get()       { return value; }
  constexpr const H& get() const { return value; }
};

template<int I, typename H>
class TupleLeaf<I, H, true> : private H {
  TupleLeaf& operator=(const TupleLeaf&);
 public:
  constexpr TupleLeaf() = default;

  template<typename T>
  constexpr explicit TupleLeaf(T&& other)
      : H(stp::forward<T>(other)) {}

  TupleLeaf(TupleLeaf const &) = default;
  TupleLeaf(TupleLeaf &&) = default;

  template<typename T>
  TupleLeaf& operator=(T&& other) {
    H::operator=(stp::forward<T>(other));
    return *this;
  }

  int swapWith(TupleLeaf& other) {
    swap(get(), other.get());
    return 0;
  }

  constexpr       H& get()       { return static_cast<H&>(*this); }
  constexpr const H& get() const { return static_cast<const H&>(*this); }
};

template<typename _Indx, typename... Ts>
class TupleImpl;

template<int... Indices, typename ...Ts>
class EMPTY_BASES_LAYOUT TupleImpl<IndexSequence<Indices...>, Ts...>
    : public TupleLeaf<Indices, Ts>... {
 private:
  template<typename...As>
  static void Swallow(As&&...) {}
 public:

  constexpr TupleImpl() = default;

  constexpr explicit TupleImpl(Ts&&... args) :
      TupleLeaf<Indices, Ts>(stp::forward<Ts>(args))... {}

  constexpr explicit TupleImpl(const Ts&... args) :
      TupleLeaf<Indices, Ts>(args)... {}

  template<typename... Us>
  constexpr explicit TupleImpl(Us&&... args) :
      TupleLeaf<Indices, Ts>(stp::forward<Us>(args))... {}

  TupleImpl(const TupleImpl& other) = default;
  TupleImpl(TupleImpl&& other) = default;

  TupleImpl& operator=(const TupleImpl& other) {
    Swallow(TupleLeaf<Indices, Ts>::operator=(static_cast<const TupleLeaf<Indices, Ts>&>(other).get())...);
    return *this;
  }

  TupleImpl& operator=(TupleImpl&& other) {
    Swallow(TupleLeaf<Indices, Ts>::operator=(stp::forward<Ts>(static_cast<TupleLeaf<Indices, Ts>&>(other).get()))...);
    return *this;
  }


  void swapWith(TupleImpl& other) {
    Swallow(TupleLeaf<Indices, Ts>::swapWith(static_cast<TupleLeaf<Indices, Ts>&>(other))...);
  }
};

#undef DECLSPEC_EMPTY_BASES

template<int N, typename T>
struct TupleEqualOp {
  bool operator()(const T& lhs, const T& rhs) {
    return TupleEqualOp<N-1, T>()(lhs, rhs) &&
           lhs.template get<N-1>() == rhs.template get<N-1>();
  }
};

template<typename T>
struct TupleEqualOp<0, T> {
  bool operator()(const T& lhs, const T& rhs) {
    return true;
  }
};

template<int N, typename T>
struct TupleLessOp {
  bool operator()(const T& lhs, const T& rhs) {
    if (lhs.template get<N-1>() < rhs.template get<N-1>())
      return true;
    if (rhs.template get<N-1>() < lhs.template get<N-1>())
      return false;
    return TupleLessOp<N-1, T>()(lhs, rhs);
  }
};

template<typename T>
struct TupleLessOp<0, T> {
  bool operator()(const T& lhs, const T& rhs) {
    return false;
  }
};

} // namespace detail

template<typename... Ts>
class Tuple {
  typedef detail::TupleImpl<MakeIndexSequence<sizeof...(Ts)>, Ts...> Base;
  Base base_;
 public:
  template<int N>
  using ElementType = TNthTypeFromParameterPack<N, Ts...>;

  constexpr Tuple() = default;

  Tuple(const Tuple&) = default;
  Tuple(Tuple&&) = default;

  constexpr explicit Tuple(const Ts& ... args) : base_(args...) {}
  constexpr explicit Tuple(Ts&& ... args) : base_(stp::forward<Ts>(args)...) {}

  template<typename... Us>
  constexpr explicit Tuple(Us&&... args) : base_(stp::forward<Us>(args)...) {}

  Tuple& operator=(const Tuple& other) {
    base_.operator=(other.base_);
    return *this;
  }

  void swapWith(Tuple& other) { base_.swapWith(other.base_); }

  static constexpr int size() { return sizeof...(Ts); }

  template<int N>
  constexpr const ElementType<N>& get() const {
    using LeafType = detail::TupleLeaf<N, ElementType<N>>;
    return static_cast<const LeafType&>(base_).get();
  }
  template<int N>
  constexpr ElementType<N>& get() {
    using LeafType = detail::TupleLeaf<N, ElementType<N>>;
    return static_cast<LeafType&>(base_).get();
  }

 private:
  template<typename F, int... I>
  auto applyImpl(F&& f, IndexSequence<I...>) -> decltype(stp::forward<F>(f)(get<I>()...)) {
    return stp::forward<F>(f)(get<I>()...);
  }
 public:

  // Given an input tuple (a1, a2, ..., an), pass the arguments of the
  // Tuple variadically to f as if by calling f(a1, a2, ..., an) and
  // return the result.
  template<typename F>
  auto apply(F&& f) -> decltype(applyImpl(stp::forward<F>(f), MakeIndexSequence<size()>())) {
    return applyImpl(stp::forward<F>(f), MakeIndexSequence<size()>());
  }

 private:
  friend bool operator==(const Tuple& lhs, const Tuple& rhs) {
    return detail::TupleEqualOp<sizeof...(Ts), Tuple>()(lhs, rhs);
  }
  friend bool operator!=(const Tuple& lhs, const Tuple& rhs) {
    return !(lhs == rhs);
  }
  friend bool operator<(const Tuple& lhs, const Tuple& rhs) {
    return detail::TupleLessOp<sizeof...(Ts), Tuple>()(lhs, rhs);
  }
  friend bool operator>(const Tuple& lhs, const Tuple& rhs) {
    return rhs < lhs;
  }
  friend bool operator<=(const Tuple& lhs, const Tuple& rhs) {
    return !(rhs < lhs);
  }
  friend bool operator>=(const Tuple& lhs, const Tuple& rhs) {
    return !(lhs < rhs);
  }
};

template<>
class Tuple<> {
 public:
  constexpr Tuple() {}

  void swapWith(Tuple&) {}

  static constexpr int size() { return 0; }
};

template<typename... Ts>
constexpr auto makeTuple(Ts&&... args) {
  return Tuple<TDecay<Ts>...>(stp::forward<Ts>(args)...);
}

namespace detail {

template<int I, typename T>
struct TupleElementHelper;

template<int I, class T0, class... Ts>
struct TupleElementHelper<I, Tuple<T0, Ts...>>
    : TupleElementHelper<I - 1, Tuple<Ts...>> { };

template<typename T0, typename... Ts>
struct TupleElementHelper<0, Tuple<T0, Ts...>> {
  typedef T0 Type;
};

} // namespace detail

template<typename T, int N>
using TupleElement = typename detail::TupleElementHelper<N, T>::Type;

} // namespace stp

#endif // STP_BASE_UTIL_TUPLE_H_
