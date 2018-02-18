// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_NULLABLE_H_
#define STP_BASE_TYPE_NULLABLE_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Comparable.h"
#include "Base/Type/FormattableFwd.h"
#include "Base/Type/Hashable.h"
#include "Base/Type/NullableFwd.h"
#include "Base/Type/Variable.h"

namespace stp {

namespace detail {

template<typename T, bool = TIsTriviallyDestructible<T>>
struct NullableValueStorage {
  // Union member must be initialized for constexpr.
  constexpr NullableValueStorage() : empty('\0') {}

  ~NullableValueStorage() {
    if (!is_valid)
      value.~T();
  }

  template<typename... TArgs>
  constexpr explicit NullableValueStorage(TArgs&&... args)
      : value(Forward<TArgs>(args)...), is_valid(true) {}

  void reset() {
    if (is_valid) {
      is_valid = false;
      value.~T();
    }
  }

  union {
    char empty;
    T value;
  };
  bool is_valid = false;
};

template<typename T>
struct NullableValueStorage<T, true> {
  constexpr NullableValueStorage() : empty('\0') {}

  ~NullableValueStorage() = default;

  template<typename... TArgs>
  constexpr explicit NullableValueStorage(TArgs&&... args)
      : value(Forward<TArgs>(args)...), is_valid(true) {}

  void reset() { is_valid = false; }

  union {
    char empty;
    T value;
  };
  bool is_valid = false;
};

} // namespace detail

template<typename T>
class NullableValue {
 public:
  // Union member must be initialized for constexpr.
  NullableValue() = default;

  template<typename U, TEnableIf<TIsConstructible<T, U&&>>* = nullptr>
  constexpr NullableValue(NullableValue<U>&& other) noexcept {
    if (other)
      relocate(other);
  }

  template<typename U, TEnableIf<TIsAssignable<T&, U&&>>* = nullptr>
  constexpr NullableValue& operator=(NullableValue<U>&& other) noexcept {
    if (other) {
      if (isValid()) {
        storage_.value = move(*other);
      } else {
        if constexpr (TsAreSame<T, U>)
          relocate(other);
        else
          init(move(*other));
      }
    } else {
      reset();
    }
    return *this;
  }

  template<typename U, TEnableIf<TIsConstructible<T, const U&>>* = nullptr>
  constexpr NullableValue(const NullableValue<U>& other) noexcept(noexcept(T(*other))) {
    if (other)
      init(*other);
  }

  template<typename U, TEnableIf<TIsAssignable<T&, const U&>>* = nullptr>
  constexpr NullableValue& operator=(const NullableValue<U>& other) noexcept(noexcept(declval<T&>() = *other)) {
    if (other) {
      if (isValid())
        storage_.value = *other;
      else
        init(*other);
    } else {
      reset();
    }
    return *this;
  }

  template<typename U, TEnableIf<TIsConstructible<T, U> && TIsConvertibleTo<U, T>>* = nullptr>
  constexpr NullableValue(U&& value) noexcept(noexcept(T(Forward<U>(value))))
      : storage_(Forward<U>(value)) {}

  template<typename U, TEnableIf<TIsConstructible<T, U> && !TIsConvertibleTo<U, T>>* = nullptr>
  constexpr explicit NullableValue(U&& value) noexcept(noexcept(T(Forward<U>(value))))
      : storage_(Forward<U>(value)) {}

  template<typename U, TEnableIf<TIsAssignable<T&, U>>* = nullptr>
  constexpr NullableValue& operator=(U&& other) noexcept(noexcept(declval<T&>() = Forward<U>(other))) {
    if (isValid())
      storage_.value = Forward<U>(other);
    else
      init(other);
    return *this;
  }

  constexpr NullableValue(nullptr_t) noexcept {}
  constexpr NullableValue& operator=(nullptr_t) noexcept { reset(); return *this; }

  friend void swap(NullableValue& l, NullableValue& r) noexcept {
    if (l.isValid() == r.isValid()) {
      if (l.isValid())
        swap(*l, *r);
    } else {
      if (l.isValid())
        r.relocate(l);
      else
        l.relocate(r);
    }
  }

  constexpr explicit operator bool() const { return storage_.is_valid; }

  constexpr const T* operator->() const { ASSERT(isValid()); return storage_.value; }
  constexpr T* operator->() { ASSERT(isValid()); return storage_.value; }

  constexpr const T& operator*() const { ASSERT(isValid()); return storage_.value; }
  constexpr T& operator*() { ASSERT(isValid()); return storage_.value; }

  friend constexpr bool operator==(const NullableValue& opt, nullptr_t) { return !opt; }
  friend constexpr bool operator==(nullptr_t, const NullableValue& opt) { return !opt; }
  friend constexpr bool operator!=(const NullableValue& opt, nullptr_t) { return opt.isValid(); }
  friend constexpr bool operator!=(nullptr_t, const NullableValue& opt) { return opt.isValid(); }

  friend constexpr int compare(const NullableValue& l, nullptr_t r) { return l ? 1 : 0; }
  friend constexpr int compare(nullptr_t l, const NullableValue& r) { return r ? -1 : 0; }

  constexpr const T* tryGet(const NullableValue& x) { return x.isValid() ? x.operator->() : nullptr; }
  constexpr T* tryGet(NullableValue& x) { return x.isValid() ? x.operator->() : nullptr; }

 private:
  detail::NullableValueStorage<T> storage_;

  template<typename U>
  constexpr void init(U&& value) {
    ASSERT(!storage_.is_valid);
    new (&storage_.value) T(Forward<U>(value));
    storage_.is_valid = true;
  }

  constexpr void reset() { storage_.reset(); }
  constexpr bool isValid() const { return storage_.is_valid; }

  void relocate(NullableValue& other) noexcept {
    ASSERT(!storage_.is_valid && other.storage_.is_valid);
    storage_.is_valid = true;
    storage_.is_valid = false;
    relocateAt(&storage_.value, other.storage_.value);
  }
};

// For big objects it could be a pessimization.
template<typename T>
struct TIsTriviallyRelocatableTmpl<NullableValue<T>>
    : TBoolConstant<TIsTriviallyRelocatable<T> && sizeof(T) <= 16> {};
// Cannot be zero-constructible - may hold big uninitialized chunk.
// Cannot be trivially equality-comparable due uninitialized data.

template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator==(const NullableValue<T>& l, const NullableValue<U>& r) {
  return l.operator bool() == r.operator bool() ? (!l || *l == *r) : false;
}
template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator!=(const NullableValue<T>& l, const NullableValue<U>& r) {
  return l.operator bool() != r.operator bool() || (l && *l != *r);
}

template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator==(const NullableValue<T>& opt, const U& value) {
  return opt ? (*opt == value) : false;
}
template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator!=(const NullableValue<T>& opt, const U& value) {
  return opt ? (*opt != value) : true;
}

template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator==(const T& value, const NullableValue<U>& opt) { return opt == value; }
template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator!=(const T& value, const NullableValue<U>& opt) { return opt != value; }

template<typename T, typename U, TEnableIf<TIsComparableWith<T, U>>* = nullptr>
constexpr int compare(const NullableValue<T>& l, const NullableValue<U>& r) {
  return l.operator bool() == r.operator bool()
      ? (l ? compare(*l, *r) : 0)
      : (l ? 1 : -1);
}

template<typename T, typename U, TEnableIf<TIsComparableWith<T, U>>* = nullptr>
constexpr int compare(const NullableValue<T>& l, const U& r) {
  return l ? compare(*l, r) : -1;
}
template<typename T, typename U, TEnableIf<TIsComparableWith<T, U>>* = nullptr>
constexpr int compare(const T& l, const NullableValue<U>& r) {
  return l ? compare(l, *r) : 1;
}

template<typename T, TEnableIf<TIsHashable<T>>* = nullptr>
constexpr HashCode partialHash(const NullableValue<T>& x) { return x ? partialHash(*x) : HashCode::Zero; }

template<typename T, TEnableIf<TIsFormattable<T>>* = nullptr>
inline TextWriter& operator<<(TextWriter& out, const NullableValue<T>& x) {
  if (x)
    out << *x;
  else
    out << nullptr;
  return out;
}
template<typename T, TEnableIf<TIsFormattable<T>>* = nullptr>
inline void format(TextWriter& out, const NullableValue<T>& x, const StringSpan& opts) {
  if (x)
    format(out, *x, opts);
  else
    out << nullptr;
}

template<typename T, typename U>
constexpr T coalesce(const NullableValue<U>& nullable, U&& default_value) {
  static_assert(TIsConvertibleTo<U, T>, "!");
  return nullable.operator bool() ? *nullable : static_cast<T>(Forward<U>(default_value));
}

template<typename T, typename U>
constexpr T coalesce(NullableValue<U>&& nullable, U&& default_value) {
  static_assert(TIsConvertibleTo<U, T>, "!");
  return nullable.operator bool() ? move(*nullable) : static_cast<T>(Forward<U>(default_value));
}

} // namespace stp

#endif // STP_BASE_TYPE_NULLABLE_H_
