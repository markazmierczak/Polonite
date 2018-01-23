// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_LISTFWD_H_
#define STP_BASE_CONTAINERS_LISTFWD_H_

#include "Base/Containers/SpanFwd.h"

namespace stp {

template<typename T>
class List;

template<typename T>
struct TIsContiguousContainerTmpl<List<T>> : TTrue {};

using String = List<char>;
using String16 = List<char16_t>;
#if SIZEOF_WCHAR_T == 2
using WString = List<wchar_t>;
#endif

} // namespace stp

#endif // STP_BASE_CONTAINERS_LISTFWD_H_
