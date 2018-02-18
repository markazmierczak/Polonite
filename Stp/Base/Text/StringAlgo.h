// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_STRINGALGO_H_
#define STP_BASE_TEXT_STRINGALGO_H_

#include "Base/Containers/Span.h"

namespace stp {

BASE_EXPORT int indexOfAny(StringSpan list, StringSpan s);

BASE_EXPORT int lastIndexOfAny(StringSpan list, StringSpan s);

BASE_EXPORT int indexOfAnyBut(StringSpan list, StringSpan s);

BASE_EXPORT int lastIndexOfAnyBut(StringSpan list, StringSpan s);

} // namespace stp

#endif // STP_BASE_TEXT_STRINGALGO_H_
