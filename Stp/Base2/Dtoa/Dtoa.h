// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_DTOA_DTOA_H_
#define STP_BASE_DTOA_DTOA_H_

#include "Base/Dtoa/DoubleConversion.h"

namespace stp {

inline bool TryParse(const char*& ptr, const char* end, double* out_value) {
  return dtoa::StringToDoubleConverter::StringToDouble(
      ptr, end, out_value);
}

constexpr int FloatToStringBufferLength = 96;
typedef char FloatToStringBuffer[FloatToStringBufferLength];

inline StringSpan FloatToString(double value, FloatToStringBuffer buffer) {
  dtoa::StringBuilder builder(buffer, FloatToStringBufferLength);
  const dtoa::DoubleToStringConverter& converter =
      dtoa::DoubleToStringConverter::EcmaScriptConverter();
  converter.ToShortest(value, &builder);
  return builder.Finalize();
}

BASE_EXPORT StringSpan FloatToFixedPrecisionString(
    FloatToStringBuffer buffer, double value, int precision);

} // namespace stp

#endif // STP_BASE_DTOA_DTOA_H_
