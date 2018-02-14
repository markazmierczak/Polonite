// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_STRINGFORMATMANY_H_
#define STP_BASE_TEXT_STRINGFORMATMANY_H_

#include "Base/Io/StringWriter.h"
#include "Base/Text/FormatMany.h"

namespace stp {

template<typename... TArgs>
inline String StringFormatMany(StringSpan fmt, const TArgs&... args) {
  String result;
  StringWriter writer(&result);
  FormatMany(writer, fmt, args...);
  return result;
}

} // namespace stp

#endif // STP_BASE_TEXT_STRINGFORMATMANY_H_
