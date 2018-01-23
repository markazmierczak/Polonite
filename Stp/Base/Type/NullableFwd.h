// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_NULLABLEFWD_H_
#define STP_BASE_TYPE_NULLABLEFWD_H_

namespace stp {

template<typename T>
class NullableValue;

template<typename T, typename = void>
struct NullableTmpl {
  typedef NullableValue<T> Type;
};

template<typename T>
struct NullableTmpl<T*> {
  typedef T* Type;
};

template<typename T>
using Nullable = typename NullableTmpl<T>::Type;

} // namespace stp

#endif // STP_BASE_TYPE_NULLABLEFWD_H_
