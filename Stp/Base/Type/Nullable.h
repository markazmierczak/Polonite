// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_NULLABLE_H_
#define STP_BASE_TYPE_NULLABLE_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/ComparableFwd.h"
#include "Base/Type/FormattableFwd.h"
#include "Base/Type/HashableFwd.h"
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

  void Reset() {
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

  void Reset() { is_valid = false; }

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
      Relocate(other);
  }

  template<typename U, TEnableIf<TIsAssignable<T&, U&&>>* = nullptr>
  constexpr NullableValue& operator=(NullableValue<U>&& other) noexcept {
    if (other) {
      if (IsValid()) {
        storage_.value = move(*other);
      } else {
        if constexpr (TsAreSame<T, U>)
          Relocate(other);
        else
          Init(move(*other));
      }
    } else {
      Reset();
    }
    return *this;
  }

  template<typename U, TEnableIf<TIsConstructible<T, const U&>>* = nullptr>
  constexpr NullableValue(const NullableValue<U>& other) noexcept(noexcept(T(*other))) {
    if (other)
      Init(*other);
  }

  template<typename U, TEnableIf<TIsAssignable<T&, const U&>>* = nullptr>
  constexpr NullableValue& operator=(const NullableValue<U>& other) noexcept(noexcept(declval<T&>() = *other)) {
    if (other) {
      if (IsValid())
        storage_.value = *other;
      else
        Init(*other);
    } else {
      Reset();
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
    if (IsValid())
      storage_.value = Forward<U>(other);
    else
      Init(other);
    return *this;
  }

  constexpr NullableValue(nullptr_t) noexcept {}
  constexpr NullableValue& operator=(nullptr_t) noexcept { Reset(); return *this; }

  friend void swap(NullableValue& l, NullableValue& r) noexcept {
    if (l.IsValid() == r.IsValid()) {
      if (l.IsValid())
        swap(*l, *r);
    } else {
      if (l.IsValid())
        r.Relocate(l);
      else
        l.Relocate(r);
    }
  }

  constexpr explicit operator bool() const { return storage_.is_valid; }

  constexpr const T* operator->() const { ASSERT(IsValid()); return storage_.value; }
  constexpr T* operator->() { ASSERT(IsValid()); return storage_.value; }

  constexpr const T& operator*() const { ASSERT(IsValid()); return storage_.value; }
  constexpr T& operator*() { ASSERT(IsValid()); return storage_.value; }

  friend constexpr bool operator==(const NullableValue& opt, nullptr_t) { return !opt; }
  friend constexpr bool operator==(nullptr_t, const NullableValue& opt) { return !opt; }
  friend constexpr bool operator!=(const NullableValue& opt, nullptr_t) { return opt.IsValid(); }
  friend constexpr bool operator!=(nullptr_t, const NullableValue& opt) { return opt.IsValid(); }

  friend constexpr int Compare(const NullableValue& l, nullptr_t r) { return l ? 1 : 0; }
  friend constexpr int Compare(nullptr_t l, const NullableValue& r) { return r ? -1 : 0; }

  constexpr const T* tryGet(const NullableValue& x) { return x.IsValid() ? x.operator->() : nullptr; }
  constexpr T* tryGet(NullableValue& x) { return x.IsValid() ? x.operator->() : nullptr; }

 private:
  detail::NullableValueStorage<T> storage_;

  template<typename U>
  constexpr void Init(U&& value) {
    ASSERT(!storage_.is_valid);
    new (&storage_.value) T(Forward<U>(value));
    storage_.is_valid = true;
  }

  constexpr void Reset() { storage_.Reset(); }
  constexpr bool IsValid() const { return storage_.is_valid; }

  void Relocate(NullableValue& other) noexcept {
    ASSERT(!storage_.is_valid && other.storage_.is_valid);
    storage_.is_valid = true;
    storage_.is_valid = false;
    RelocateAt(&storage_.value, other.storage_.value);
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
constexpr int Compare(const NullableValue<T>& l, const NullableValue<U>& r) {
  return l.operator bool() == r.operator bool()
      ? (l ? Compare(*l, *r) : 0)
      : (l ? 1 : -1);
}

template<typename T, typename U, TEnableIf<TIsComparableWith<T, U>>* = nullptr>
constexpr int Compare(const NullableValue<T>& l, const U& r) {
  return l ? Compare(*l, r) : -1;
}
template<typename T, typename U, TEnableIf<TIsComparableWith<T, U>>* = nullptr>
constexpr int Compare(const T& l, const NullableValue<U>& r) {
  return l ? Compare(l, *r) : 1;
}

template<typename T, TEnableIf<TIsHashable<T>>* = nullptr>
constexpr HashCode Hash(const NullableValue<T>& x) { return x ? Hash(*x) : HashCode::Zero; }

template<typename T, TEnableIf<TIsFormattable<T>>* = nullptr>
inline TextWriter& operator<<(TextWriter& out, const NullableValue<T>& x) {
  if (x)
    out << *x;
  else
    out << nullptr;
  return out;
}
template<typename T, TEnableIf<TIsFormattable<T>>* = nullptr>
inline void Format(TextWriter& out, const NullableValue<T>& x, const StringSpan& opts) {
  if (x)
    Format(out, *x, opts);
  else
    out << nullptr;
}

template<typename T, typename U>
constexpr T Coalesce(const NullableValue<U>& nullable, U&& default_value) {
  static_assert(TIsConvertibleTo<U, T>, "!");
  return nullable.operator bool() ? *nullable : static_cast<T>(Forward<U>(default_value));
}

template<typename T, typename U>
constexpr T Coalesce(NullableValue<U>&& nullable, U&& default_value) {
  static_assert(TIsConvertibleTo<U, T>, "!");
  return nullable.operator bool() ? move(*nullable) : static_cast<T>(Forward<U>(default_value));
}

} // namespace stp

#endif // STP_BASE_TYPE_NULLABLE_H_
