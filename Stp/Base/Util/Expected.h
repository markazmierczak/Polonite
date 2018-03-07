// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_EXPECTED_H_
#define STP_BASE_UTIL_EXPECTED_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Variable.h"

namespace stp {

namespace detail {
namespace expected {

constexpr enum class ValueTag {} HasValue {};
constexpr enum class ErorTag {} HasError {};

template<class T, class E>
union ConstexprUnion {
  typedef T ValueType;
  typedef E ErrorType;
  char dummy;
  ValueType val;
  ErrorType err;
  constexpr ConstexprUnion() : dummy() {}
  constexpr ConstexprUnion(ValueTag) : val() {}
  constexpr ConstexprUnion(ErorTag) : err() {}
  constexpr ConstexprUnion(ValueTag, const ValueType& v) : val(v) {}
  constexpr ConstexprUnion(ErorTag, const ErrorType& e) : err(e) {}
  ~ConstexprUnion() = default;
};

template<class T, class E>
union Union {
  typedef T ValueType;
  typedef E ErrorType;
  char dummy;
  ValueType val;
  ErrorType err;
  constexpr Union() : dummy() {}
  constexpr Union(ValueTag) : val() {}
  constexpr Union(ErorTag) : err() {}
  constexpr Union(ValueTag, const ValueType& val) : val(val) {}
  constexpr Union(ValueTag, ValueType&& val) : val(stp::move(val)) {}
  constexpr Union(ErorTag, const ErrorType& err) : err(err) {}
  constexpr Union(ErorTag, ErrorType&& err) : err(stp::move(err)) {}
  ~Union() {}
};

template<class E>
union ConstexprUnion<void, E> {
  typedef void ValueType;
  typedef E ErrorType;
  char dummy;
  ErrorType err;
  constexpr ConstexprUnion() : dummy() {}
  constexpr ConstexprUnion(ValueTag) : dummy() {}
  constexpr ConstexprUnion(ErorTag) : err() {}
  constexpr ConstexprUnion(ErorTag, const ErrorType& e) : err(e) {}
  ~ConstexprUnion() = default;
};

template<class E>
union Union<void, E> {
  typedef void ValueType;
  typedef E ErrorType;
  char dummy;
  ErrorType err;
  constexpr Union() : dummy() {}
  constexpr Union(ValueTag) : dummy() {}
  constexpr Union(ErorTag) : err() {}
  constexpr Union(ErorTag, const ErrorType& err) : err(err) {}
  constexpr Union(ErorTag, ErrorType&& err) : err(stp::move(err)) {}
  ~Union() {}
};

template<class T, class E>
struct ConstexprBase {
  typedef T ValueType;
  typedef E ErrorType;
  ConstexprUnion<ValueType, ErrorType> s;
  bool has;
  constexpr ConstexprBase() : s(), has(true) {}
  constexpr ConstexprBase(ValueTag tag) : s(tag), has(true) {}
  constexpr ConstexprBase(ErorTag tag) : s(tag), has(false) {}
  constexpr ConstexprBase(ValueTag tag, const ValueType& val) : s(tag, val), has(true) {}
  constexpr ConstexprBase(ErorTag tag, const ErrorType& err) : s(tag, err), has(false) {}
  ~ConstexprBase() = default;
};

template<class T, class E>
struct Base {
  typedef T ValueType;
  typedef E ErrorType;
  Union<ValueType, ErrorType> s;
  bool has;
  constexpr Base() : s(), has(true) {}
  constexpr Base(ValueTag tag) : s(tag), has(true) {}
  constexpr Base(ErorTag tag) : s(tag), has(false) {}
  constexpr Base(ValueTag tag, const ValueType& val) : s(tag, val), has(true) {}
  constexpr Base(ValueTag tag, ValueType&& val) : s(tag, stp::move(val)), has(true) {}
  constexpr Base(ErorTag tag, const ErrorType& err) : s(tag, err), has(false) {}
  constexpr Base(ErorTag tag, ErrorType&& err) : s(tag, stp::move(err)), has(false) {}

  Base(const Base& o) : has(o.has) {
    if (has)
      ::new (&s.val) ValueType(o.s.val);
    else
      ::new (&s.err) ErrorType(o.s.err);
  }

  Base(Base&& o) : has(o.has) {
    if (has)
      ::new (&s.val) ValueType(stp::move(o.s.val));
    else
      ::new (&s.err) ErrorType(stp::move(o.s.err));
  }

  ~Base() {
    if (has)
      destroyObject(s.val);
    else
      destroyObject(s.err);
  }
};

template<class E>
struct ConstexprBase<void, E> {
  typedef void ValueType;
  typedef E ErrorType;
  ConstexprUnion<ValueType, ErrorType> s;
  bool has;
  constexpr ConstexprBase() : s(), has(true) {}
  constexpr ConstexprBase(ValueTag tag) : s(tag), has(true) {}
  constexpr ConstexprBase(ErorTag tag) : s(tag), has(false) {}
  constexpr ConstexprBase(ErorTag tag, const ErrorType& err) : s(tag, err), has(false) {}
  constexpr ConstexprBase(ErorTag tag, ErrorType&& err) : s(tag, stp::move(err)), has(false) {}
  ~ConstexprBase() = default;
};

template<class E>
struct Base<void, E> {
  typedef void ValueType;
  typedef E ErrorType;
  Union<ValueType, ErrorType> s;
  bool has;
  constexpr Base() : s(), has(true) {}
  constexpr Base(ValueTag tag) : s(tag), has(true) {}
  constexpr Base(ErorTag tag) : s(tag), has(false) {}
  constexpr Base(ErorTag tag, const ErrorType& err) : s(tag, err), has(false) {}
  constexpr Base(ErorTag tag, ErrorType&& err) : s(tag, stp::move(err)), has(false) {}
  Base(const Base& o) : has(o.has) {
    if (!has)
      ::new (&s.err) ErrorType(o.s.err);
  }
  Base(Base&& o) : has(o.has) {
    if (!has)
      ::new (&s.err) ErrorType(stp::move(o.s.err));
  }
  ~Base() {
    if (!has)
      destroyObject(s.err);
  }
};

template<class T, class E>
using BaseSelect = TConditional<
    ((TIsVoid<T> || TIsTriviallyDestructible<T>) && TIsTriviallyDestructible<E>),
    ConstexprBase<T, E>,
    Base<T, E>
>;

} // namespace expected
} // namespace detail

template<class T, class E>
class [[nodiscard]] Expected : private detail::expected::BaseSelect<T, E> {
  typedef detail::expected::BaseSelect<T, E> Base;

 public:
  typedef typename Base::ValueType ValueType;
  typedef typename Base::ErrorType ErrorType;

  constexpr Expected() : Base(detail::expected::HasValue) {}
  Expected(const Expected&) = default;
  Expected(Expected&&) = default;
  ~Expected() = default;

  Expected& operator=(const Expected& e) { exchange(*this, e); return *this; }
  Expected& operator=(Expected&& e) { exchange(*this, stp::move(e)); return *this; }
  template<class U> Expected& operator=(U&& u) { exchange(*this, stp::forward<E>(u)); return *this; }

  constexpr Expected(const ValueType& v) : Base(detail::expected::HasValue, v) {}
  constexpr Expected(ValueType&& v) : Base(detail::expected::HasValue, stp::move(v)) {}
  constexpr Expected(const ErrorType& e) : Base(detail::expected::HasError, e) {}
  constexpr Expected(ErrorType&& e) : Base(detail::expected::HasError, stp::move(e)) {}

  friend void swap(Expected& l, Expected& r) {
    if (l.has && r.has) {
      swap(l.s.val, r.s.val);
    } else if (l.has && !r.has) {
      ErrorType e(stp::move(r.s.err));
      destroyObject(r.s.err);
      ::new (&r.s.val) ValueType(stp::move(l.s.val));
      destroyObject(l.s.val);
      ::new (&l.s.err) ErrorType(stp::move(e));
      swap(l.has, r.has);
    } else if (!l.has && r.has) {
      ValueType v(stp::move(r.s.val));
      destroyObject(r.s.val);
      ::new (&r.s.err) ErrorType(stp::move(l.s.err));
      destroyObject(l.s.err);
      ::new (&l.s.val) ValueType(stp::move(v));
      swap(l.has, r.has);
    } else {
      swap(l.s.err, r.s.err);
    }
  }

  constexpr explicit operator bool() const { return Base::has; }
  constexpr bool hasValue() const { return Base::has; }
  constexpr const ValueType& getValue() const & { ASSERT(Base::has); return Base::s.val; }
  constexpr const ValueType&& getValue() const && { ASSERT(Base::has); return stp::move(Base::s.val); }
  constexpr ValueType& getValue() & { ASSERT(Base::has); return Base::s.val; }
  constexpr ValueType&& getValue() && { ASSERT(Base::has); return stp::move(Base::s.val); }
  constexpr const ErrorType& getError() const & { ASSERT(!Base::has); return Base::s.err; }
  constexpr const ErrorType&& getError() const && { ASSERT(!Base::has); return stp::move(Base::s.err); }
  constexpr ErrorType& getError() & { ASSERT(!Base::has); return Base::s.err; }
  constexpr ErrorType&& getError() && { ASSERT(!Base::has); return stp::move(Base::s.err); }
};

template<class E>
class [[nodiscard]] Expected<void, E> : private detail::expected::BaseSelect<void, E> {
  typedef detail::expected::BaseSelect<void, E> Base;

 public:
  typedef typename Base::ValueType ValueType;
  typedef typename Base::ErrorType ErrorType;

  constexpr Expected() : Base(detail::expected::HasValue) {}
  Expected(const Expected&) = default;
  Expected(Expected&&) = default;
  ~Expected() = default;

  Expected& operator=(const Expected& e) { exchange(*this, e); return *this; }
  Expected& operator=(Expected&& e) { exchange(*this, stp::move(e)); return *this; }

  constexpr Expected(const ErrorType& e) : Base(detail::expected::HasError, e) {}
  constexpr Expected(ErrorType&& e) : Base(detail::expected::HasError, stp::move(e)) {}

  friend void swap(Expected& l, Expected& r) {
    if (l.has && r.has) {
      // nothing to do
    } else if (l.has && !r.has) {
      ErrorType e(stp::move(r.s.err));
      ::new (&l.s.err) ErrorType(e);
      swap(l.has, r.has);
    } else if (!l.has && r.has) {
      ::new (&r.s.err) ErrorType(stp::move(l.s.err));
      swap(l.has, r.has);
    } else {
      swap(l.s.err, r.s.err);
    }
  }

  constexpr explicit operator bool() const { return Base::has; }
  constexpr bool hasValue() const { return Base::has; }
  constexpr void getValue() const { ASSERT(Base::has); return void(); }
  constexpr const E& getError() const & { ASSERT(!Base::has); return Base::s.err; }
  constexpr const E&& getError() const && { ASSERT(!Base::has); return stp::move(Base::s.err); }
  constexpr E& getError() & { ASSERT(!Base::has); return Base::s.err; }
  constexpr E&& getError() && { ASSERT(!Base::has); return stp::move(Base::s.err); }
};

template<class T, class E> constexpr bool operator==(const Expected<T, E>& x, const Expected<T, E>& y) { return bool(x) == bool(y) && (x ? x.value() == y.value() : x.error() == y.error()); }
template<class T, class E> constexpr bool operator!=(const Expected<T, E>& x, const Expected<T, E>& y) { return !(x == y); }

template<class E> constexpr bool operator==(const Expected<void, E>& x, const Expected<void, E>& y) { return bool(x) == bool(y) && (x ? true : x.error() == y.error()); }

template<class T, class E> constexpr bool operator==(const Expected<T, E>& x, const T& y) { return x == Expected<T, E>(y); }
template<class T, class E> constexpr bool operator==(const T& x, const Expected<T, E>& y) { return Expected<T, E>(x) == y; }
template<class T, class E> constexpr bool operator!=(const Expected<T, E>& x, const T& y) { return x != Expected<T, E>(y); }
template<class T, class E> constexpr bool operator!=(const T& x, const Expected<T, E>& y) { return Expected<T, E>(x) != y; }

template<class T, class E>
constexpr T expect(Expected<T, E>&& x, const char* msg = "") {
  PANIC_IF(!x, msg);
  return stp::move(x.getValue());
}

template<class T, class E>
constexpr T expectOrAssert(Expected<T, E>&& x, const char* msg = "") {
  ASSERT(x, msg);
  return stp::move(x.getValue());
}

} // namespace stp

#endif // STP_BASE_UTIL_EXPECTED_H_
