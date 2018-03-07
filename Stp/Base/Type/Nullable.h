// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_NULLABLE_H_
#define STP_BASE_TYPE_NULLABLE_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Variable.h"

namespace stp {

namespace detail {

template<typename T, bool = TIsTriviallyDestructible<T>>
struct NullableStorage {
  // Union member must be initialized for constexpr.
  constexpr NullableStorage() : empty('\0') {}

  ~NullableStorage() {
    if (!is_valid)
      value.~T();
  }

  template<typename... TArgs>
  constexpr explicit NullableStorage(TArgs&&... args)
      : value(forward<TArgs>(args)...), is_valid(true) {}

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
struct NullableStorage<T, true> {
  constexpr NullableStorage() : empty('\0') {}

  ~NullableStorage() = default;

  template<typename... TArgs>
  constexpr explicit NullableStorage(TArgs&&... args)
      : value(forward<TArgs>(args)...), is_valid(true) {}

  void reset() { is_valid = false; }

  union {
    char empty;
    T value;
  };
  bool is_valid = false;
};

} // namespace detail

template<typename T>
class Nullable {
 public:
  // Union member must be initialized for constexpr.
  Nullable() = default;

  constexpr Nullable(nullptr_t) {}
  constexpr Nullable& operator=(nullptr_t) { reset(); return *this; }

  constexpr Nullable(Nullable&& other) {
    if (other)
      relocate(other);
  }

  constexpr Nullable& operator=(Nullable&& other) {
    if (other) {
      if (*this) {
        storage_.value = move(*other);
      } else {
        relocate(other);
      }
    } else {
      reset();
    }
    return *this;
  }

  constexpr Nullable(const Nullable& other) {
    if (other)
      init(*other);
  }

  constexpr Nullable& operator=(const Nullable& other) {
    if (other) {
      if (*this)
        storage_.value = *other;
      else
        init(*other);
    } else {
      reset();
    }
    return *this;
  }

  constexpr Nullable(T&& value)
      : storage_(move(value)) {}

  constexpr Nullable(const T& value)
      : storage_(value) {}

  constexpr Nullable& operator=(T&& other) {
    if (isNull())
      init(move(other));
    else
      storage_.value = move(other);
    return *this;
  }

  constexpr Nullable& operator=(const T& other) noexcept(noexcept(declval<T&>() = other)) {
    if (isNull())
      init(other);
    else
      storage_.value = other;
    return *this;
  }

  friend void swap(Nullable& l, Nullable& r) noexcept {
    if (l.isNull() == r.isNull()) {
      if (l)
        swap(*l, *r);
    } else {
      if (l)
        r.relocate(l);
      else
        l.relocate(r);
    }
  }

  constexpr const T* operator->() const { ASSERT(!isNull()); return get(); }
  constexpr T* operator->() { ASSERT(!isNull()); return get(); }

  constexpr const T& operator*() const { ASSERT(!isNull()); return *get(); }
  constexpr T& operator*() { ASSERT(!isNull()); return *get(); }

  constexpr bool isNull() const { return !storage_.is_valid; }
  constexpr explicit operator bool() const { return !isNull(); }
  constexpr bool operator!() const { return isNull(); }

  constexpr const T* get() const {
    return storage_.is_valid ? &storage_.value : nullptr;
  }
  constexpr T* get() {
    return storage_.is_valid ? &storage_.value : nullptr;
  }

  constexpr void reset() { storage_.reset(); }

  constexpr T take() {
    ASSERT(!isNull());
    T result = move(storage_.value);
    storage_.is_valid = false;
    destroyObject(storage_.value);
    return result;
  }

 private:
  detail::NullableStorage<T> storage_;

  template<typename U>
  constexpr void init(U&& value) {
    ASSERT(!storage_.is_valid);
    new (&storage_.value) T(forward<U>(value));
    storage_.is_valid = true;
  }

  void relocate(Nullable& other) noexcept {
    ASSERT(!storage_.is_valid && other.storage_.is_valid);
    storage_.is_valid = true;
    storage_.is_valid = false;
    relocateObject(&storage_.value, other.storage_.value);
  }
};

template<typename T>
struct TIsTriviallyRelocatableTmpl<Nullable<T>>
    : TBoolConstant<TIsTriviallyRelocatable<T>> {};
// Cannot be zero-constructible - may hold big uninitialized chunk.
// Cannot be trivially equality-comparable due uninitialized data.

template<typename T>
constexpr bool operator==(const Nullable<T>& x, nullptr_t) { return !x; }
template<typename T>
constexpr bool operator==(nullptr_t, const Nullable<T>& x) { return !x; }
template<typename T>
constexpr bool operator!=(const Nullable<T>& x, nullptr_t) { return !!x; }
template<typename T>
constexpr bool operator!=(nullptr_t, const Nullable<T>& x) { return !!x; }
template<typename T>
constexpr int compare(const Nullable<T>& x, nullptr_t) { return x ? 1 : 0; }
template<typename T>
constexpr int compare(nullptr_t, const Nullable<T>& x) { return x ? -1 : 0; }

template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator==(const Nullable<T>& l, const Nullable<U>& r) {
  return l.isNull() == r.isNull() ? (!l || *l == *r) : false;
}
template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator!=(const Nullable<T>& l, const Nullable<U>& r) {
  return l.isNull() != r.isNull() || (l && *l != *r);
}

template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator==(const Nullable<T>& opt, const U& value) {
  return opt ? (*opt == value) : false;
}
template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator!=(const Nullable<T>& opt, const U& value) {
  return opt ? (*opt != value) : true;
}

template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator==(const T& value, const Nullable<U>& opt) { return opt == value; }
template<typename T, typename U, TEnableIf<TIsEqualityComparableWith<T, U>>* = nullptr>
constexpr bool operator!=(const T& value, const Nullable<U>& opt) { return opt != value; }

template<typename T, typename U, typename X>
constexpr T coalesce(const Nullable<U>& nullable, X&& default_value) {
  static_assert(TIsConvertibleTo<U, T>, "!");
  return nullable ? T(*nullable) : T(forward<X>(default_value));
}

template<typename T, typename U, typename X>
constexpr T coalesce(Nullable<U>&& nullable, X&& default_value) {
  static_assert(TIsConvertibleTo<U, T>, "!");
  return nullable ? T(move(*nullable)) : T(forward<X>(default_value));
}

} // namespace stp

#endif // STP_BASE_TYPE_NULLABLE_H_
