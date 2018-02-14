// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_FORMATMANYFWD_H_
#define STP_BASE_TEXT_FORMATMANYFWD_H_

#include "Base/Containers/ListFwd.h"

namespace stp {

class TextWriter;

template<typename... TArgs>
inline void FormatMany(TextWriter& out, StringSpan fmt, const TArgs&... args);

} // namespace stp

#endif // STP_BASE_TEXT_FORMATMANYFWD_H_
