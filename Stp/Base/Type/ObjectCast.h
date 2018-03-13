// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_OBJECTCAST_H_
#define STP_BASE_TYPE_OBJECTCAST_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/CVRef.h"

namespace stp {

// The core of the implementation of isInstanceOf<X> is here; To and From
// should be the names of classes. This template can be specialized to customize
// the implementation of isInstanceOf<> without rewriting it from scratch.
template<typename TTo, typename TFrom, typename TEnabler = void>
struct TIsInstanceOf;

template<class T, class U>
inline bool isInstanceOf(U& x) noexcept {
  const U& y = x;
  return TIsBaseOf<T, U> || TIsInstanceOf<T, U>::check(y);
}

template<class T, class U>
inline bool isInstanceOf(U* x) noexcept {
  const U& y = *x;
  return TIsBaseOf<T, U> || TIsInstanceOf<T, U>::check(y);
}

template<class T, class U, class R = TApplyCV<U, T>>
inline R& objectCast(U& x) noexcept {
  ASSERT(isInstanceOf<T>(x), "argument of incompatible type!");
  return static_cast<R&>(x);
}

template<class T, class U, class R = TApplyCV<U, T>>
inline R* objectCast(U* x) noexcept {
  ASSERT(!x || isInstanceOf<T>(*x), "argument of incompatible type!");
  return static_cast<R*>(x);
}

template<class T, class U, class R = TApplyCV<U, T>>
inline R* tryObjectCast(U& x) noexcept {
  return isInstanceOf<T>(x) ? static_cast<R*>(&x) : nullptr;
}

template<class T, class U, class R = TApplyCV<U, T>>
inline R* tryObjectCast(U* x) noexcept {
  return x ? stp::tryObjectCast<T>(*x) : nullptr;
}

} // namespace stp

#endif // STP_BASE_TYPE_OBJECTCAST_H_
