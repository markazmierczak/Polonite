// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_PARSEFLOAT_H_
#define STP_BASE_TYPE_PARSEFLOAT_H_

#include "Base/Dtoa/Dtoa.h"
#include "Base/Text/StringSpan.h"
#include "Base/Type/Scalar.h"

namespace stp {

// For floating-point conversions, only conversions of input strings in decimal
// form are defined to work. Behavior with strings representing floating-point
// numbers in hexadecimal, and strings representing non-finite values (such as
// NaN and inf) is undefined. Otherwise, these behave the same as the integral
// variants.  This expects the input string to NOT be specific to the locale.
// If your input is locale specific, use ICU to read the number.
template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
bool TryParse(StringSpan input, T& output) {
  const char* it = begin(input);
  const char* it_end = end(input);

  bool ok = TryParse(it, it_end, &output);
  if (ok)
    ok = it == it_end;
  return ok;
}

} // namespace stp

#endif // STP_BASE_TYPE_PARSEFLOAT_H_
