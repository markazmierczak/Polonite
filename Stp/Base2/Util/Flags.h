// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_FLAGS_H_
#define STP_BASE_UTIL_FLAGS_H_

#include "Base/Type/Basic.h"

namespace stp {

template<typename T>
struct TIsFlagsEnumTmpl : TFalse {};

template<typename T>
constexpr bool TIsFlagsEnum = TIsFlagsEnumTmpl<T>::Value;

template<typename T, typename U = TUnderlying<T>>
class Flags final {
 public:
  static_assert(TIsEnum<T>, "!");

  typedef T EnumType;
  typedef U MaskType;

  constexpr Flags() : mask_(0) {}

  constexpr Flags(EnumType flag) : mask_(static_cast<MaskType>(flag)) {}

  constexpr explicit Flags(MaskType mask) : mask_(static_cast<MaskType>(mask)) {}

  constexpr ALWAYS_INLINE MaskType bits() const { return mask_; }

  constexpr bool operator==(EnumType flag) const { return mask_ == static_cast<MaskType>(flag); }
  constexpr bool operator!=(EnumType flag) const { return mask_ != static_cast<MaskType>(flag); }

  constexpr void Set(EnumType flag) { mask_ |= static_cast<MaskType>(flag); }
  constexpr void Unset(EnumType flag) { mask_ &= ~static_cast<MaskType>(flag); }
  constexpr void Toggle(EnumType flag) { mask_ ^= static_cast<MaskType>(flag); }

  constexpr void Set(Flags other) { mask_ |= other.mask_; }
  constexpr void Unset(Flags other) { mask_ &= ~other.mask_; }
  constexpr void Toggle(Flags other) { mask_ ^= other.mask_; }

  constexpr bool IsZero() const { return mask_ == 0; }
  constexpr void Clear() { mask_ = 0; }

  constexpr bool Have(EnumType flag) const {
    return (mask_ & static_cast<MaskType>(flag)) != 0;
  }
  constexpr bool HaveAnyOf(Flags other) const {
    return (mask_ & other.mask_) != 0;
  }
  constexpr bool HaveAllOf(Flags other) const {
    return (mask_ & other.mask_) == other.mask_;
  }

  constexpr Flags& operator&=(const Flags& flags) {
    mask_ &= flags.mask_;
    return *this;
  }
  constexpr Flags& operator|=(const Flags& flags) {
    mask_ |= flags.mask_;
    return *this;
  }
  constexpr Flags& operator^=(const Flags& flags) {
    mask_ ^= flags.mask_;
    return *this;
  }

  constexpr Flags operator&(const Flags& flags) const { return Flags(*this) &= flags; }
  constexpr Flags operator|(const Flags& flags) const { return Flags(*this) |= flags; }
  constexpr Flags operator^(const Flags& flags) const { return Flags(*this) ^= flags; }

  constexpr Flags& operator&=(EnumType flag) { return operator&=(Flags(flag)); }
  constexpr Flags& operator|=(EnumType flag) { return operator|=(Flags(flag)); }
  constexpr Flags& operator^=(EnumType flag) { return operator^=(Flags(flag)); }

  constexpr Flags operator&(EnumType flag) const { return operator&(Flags(flag)); }
  constexpr Flags operator|(EnumType flag) const { return operator|(Flags(flag)); }
  constexpr Flags operator^(EnumType flag) const { return operator^(Flags(flag)); }

  constexpr Flags operator~() const { return Flags(~mask_); }

  constexpr operator MaskType() const { return mask_; }
  constexpr explicit operator EnumType() const { return static_cast<EnumType>(mask_); }
  constexpr bool operator!() const { return !mask_; }

  constexpr bool operator==(const Flags& other) const { return mask_ == other.mask_; }
  constexpr bool operator!=(const Flags& other) const { return !operator==(other); }

  friend constexpr Flags operator&(EnumType lhs, Flags rhs) { return rhs & lhs; }
  friend constexpr Flags operator|(EnumType lhs, Flags rhs) { return rhs | lhs; }
  friend constexpr Flags operator^(EnumType lhs, Flags rhs) { return rhs ^ lhs; }

 private:
  MaskType mask_;
};

template<typename E, TEnableIf<TIsFlagsEnum<E>>* = nullptr>
constexpr Flags<E> operator&(E l, E r) { return Flags<E>(l) & r; }

template<typename E, TEnableIf<TIsFlagsEnum<E>>* = nullptr>
constexpr Flags<E> operator|(E l, E r) { return Flags<E>(l) | r; }

template<typename E, TEnableIf<TIsFlagsEnum<E>>* = nullptr>
constexpr Flags<E> operator^(E l, E r) { return Flags<E>(l) ^ r; }

template<typename E, TEnableIf<TIsFlagsEnum<E>>* = nullptr>
constexpr Flags<E> operator~(E x) { return ~Flags<E>(x); }

} // namespace stp

#endif // STP_BASE_UTIL_FLAGS_H_
