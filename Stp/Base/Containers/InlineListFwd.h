// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_INLINELISTFWD_H_
#define STP_BASE_CONTAINERS_INLINELISTFWD_H_

#include "Base/Containers/SpanFwd.h"

namespace stp {

template<typename T>
class InlineListBase;

template<typename T, int N>
class InlineList;

template<typename T>
struct TIsContiguousContainerTmpl<InlineListBase<T>> : TTrue {};

template<typename T, int N>
struct TIsContiguousContainerTmpl<InlineList<T, N>> : TTrue {};

template<int N>
using InlineString = InlineList<char, N>;

} // namespace stp

#endif // STP_BASE_CONTAINERS_INLINELISTFWD_H_
