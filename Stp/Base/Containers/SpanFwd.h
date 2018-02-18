// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_SPANFWD_H_
#define STP_BASE_CONTAINERS_SPANFWD_H_

#include "Base/Type/CVRef.h"

namespace stp {

template<typename T>
class Span;
template<typename T>
class MutableSpan;

using StringSpan = Span<char>;
using String16Span = Span<char16_t>;

using MutableStringSpan = MutableSpan<char>;
using MutableString16Span = MutableSpan<char16_t>;

#if SIZEOF_WCHAR_T == 2
using WStringSpan = Span<wchar_t>;
using MutableWStringSpan = MutableSpan<wchar_t>;
#endif

} // namespace stp

#endif // STP_BASE_CONTAINERS_SPANFWD_H_
