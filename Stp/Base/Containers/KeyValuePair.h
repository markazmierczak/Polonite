// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_KEYVALUEPAIR_H_
#define STP_BASE_CONTAINERS_KEYVALUEPAIR_H_

#include "Base/Type/Variable.h"

namespace stp {

template<typename K, typename T>
class KeyValuePair {
 public:
  template<typename TKey, typename TValue>
  KeyValuePair(TKey&& key, TValue&& value)
      : key_(Forward<TKey>(key)), value_(Forward<TValue>(value)) {}

  KeyValuePair(KeyValuePair&& o) noexcept : key_(Move(o.key_)), value_(Move(o.value_)) {}
  KeyValuePair& operator=(KeyValuePair&& o) noexcept;

  const K& key() const { return key_; }
  const T& value() const { return value_; }
  T& value() { return value_; }

  friend void Swap(KeyValuePair& l, KeyValuePair& r) noexcept {
    Swap(l.key_, r.key_);
    Swap(l.value_, r.value_);
  }
  friend bool operator==(const KeyValuePair& l, const KeyValuePair& r) {
    return l.key_ == r.key_ && l.value_ == r.value_;
  }
  friend bool operator!=(const KeyValuePair& l, const KeyValuePair& r) { return !operator==(l, r); }
  friend int Compare(const KeyValuePair& l, const KeyValuePair& r) { return Compare(l.key_, r.key_); }

 private:
  K key_;
  T value_;
};

template<typename K, typename T>
struct TIsZeroConstructibleTmpl<KeyValuePair<K, T>>
    : TBoolConstant<TIsZeroConstructible<K> && TIsZeroConstructible<T>> {};

template<typename K, typename T>
struct TIsTriviallyRelocatableTmpl<KeyValuePair<K, T>>
    : TBoolConstant<TIsTriviallyRelocatable<K> && TIsTriviallyRelocatable<T>> {};

template<typename K, typename T>
struct TIsTriviallyEqualityComparableTmpl<KeyValuePair<K, T>>
    : TBoolConstant<TIsTriviallyEqualityComparable<K> &&
                    TIsTriviallyEqualityComparable<T> &&
                    sizeof(KeyValuePair<K, T>) == sizeof(K) + sizeof(T)> {};

template<typename K, typename T>
inline KeyValuePair<K, T>& KeyValuePair<K, T>::operator=(KeyValuePair&& o) noexcept {
  key_ = Move(o.key_);
  value_ = Move(o.value_);
  return *this;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_KEYVALUEPAIR_H_
