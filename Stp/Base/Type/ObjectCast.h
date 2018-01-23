// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_OBJECTCAST_H_
#define STP_BASE_TYPE_OBJECTCAST_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/CVRef.h"

namespace stp {

// The core of the implementation of IsInstanceOf<X> is here; To and From
// should be the names of classes. This template can be specialized to customize
// the implementation of IsInstanceOf<> without rewriting it from scratch.
template<typename TTo, typename TFrom, typename TEnabler = void>
struct TIsInstanceOf;

template<class T, class U>
inline bool IsInstanceOf(U& x) {
  const U& y = x;
  return TIsBaseOf<T, U> || TIsInstanceOf<T, U>::Check(y);
}

template<class T, class U>
inline bool IsInstanceOf(U* x) {
  const U& y = *x;
  return TIsBaseOf<T, U> || TIsInstanceOf<T, U>::Check(y);
}

template<class T, class U, class R = TApplyCV<U, T>>
inline R& ObjectCast(U& x) {
  ASSERT(IsInstanceOf<T>(x), "argument of incompatible type!");
  return static_cast<R&>(x);
}

template<class T, class U, class R = TApplyCV<U, T>>
inline R* ObjectCast(U* x) {
  ASSERT(!x || IsInstanceOf<T>(*x), "argument of incompatible type!");
  return static_cast<R*>(x);
}

template<class T, class U, class R = TApplyCV<U, T>>
inline R* TryObjectCast(U& x) {
  return IsInstanceOf<T>(x) ? static_cast<R*>(&x) : nullptr;
}

template<class T, class U, class R = TApplyCV<U, T>>
inline R* TryObjectCast(U* x) {
  return x ? stp::TryObjectCast<T>(*x) : nullptr;
}

} // namespace stp

#endif // STP_BASE_TYPE_OBJECTCAST_H_
