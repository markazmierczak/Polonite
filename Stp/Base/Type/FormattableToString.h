// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_FORMATTABLETOSTRING_H_
#define STP_BASE_TYPE_FORMATTABLETOSTRING_H_

#include "Base/Io/StringWriter.h"
#include "Base/Type/Formattable.h"

namespace stp {

template<typename TValue>
inline String FormattableToString(const TValue& value) {
  String result;
  StringWriter writer(&result);
  writer << value;
  return result;
}

template<typename TValue>
inline String FormattableToString(const TValue& value, const StringSpan& opts) {
  String result;
  StringWriter writer(&result);
  Format(writer, value, opts);
  return result;
}

} // namespace stp

#endif // STP_BASE_TYPE_FORMATTABLETOSTRING_H_
