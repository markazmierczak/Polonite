// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_ARRAYFWD_H_
#define STP_BASE_CONTAINERS_ARRAYFWD_H_

#include "Base/Containers/SpanFwd.h"

namespace stp {

template<typename T, int N>
struct Array;

template<typename T, int N>
struct TIsContiguousContainerTmpl<Array<T, N>> : TTrue {};

} // namespace stp

#endif // STP_BASE_CONTAINERS_ARRAYFWD_H_
