// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2010 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "Base/Dtoa/DoubleConversion.h"

#include "Base/Dtoa/BignumDtoa.h"
#include "Base/Dtoa/Double.h"
#include "Base/Dtoa/FastDtoa.h"
#include "Base/Dtoa/FixedDtoa.h"
#include "Base/Dtoa/Strtod.h"
#include "Base/Dtoa/Utils.h"

#include <limits.h>
#include <math.h>

namespace stp {

namespace dtoa {

const DoubleToStringConverter& DoubleToStringConverter::EcmaScriptConverter() {
  int flags = UniqueZero | EmitPositiveExponentSign;
  static DoubleToStringConverter converter(
      flags,
      "Infinity", "NaN", 'e',
      -6, 21, 6, 0);
  return converter;
}


bool DoubleToStringConverter::HandleSpecialValues(
    double value,
    StringBuilder* result_builder) const {
  Double double_inspect(value);
  if (double_inspect.IsInfinite()) {
    if (infinity_symbol_ == NULL) return false;
    if (value < 0) {
      result_builder->AddCharacter('-');
    }
    result_builder->AddString(infinity_symbol_);
    return true;
  }
  if (double_inspect.isNaN()) {
    if (nan_symbol_ == NULL) return false;
    result_builder->AddString(nan_symbol_);
    return true;
  }
  return false;
}


void DoubleToStringConverter::CreateExponentialRepresentation(
    const char* decimal_digits,
    int length,
    int exponent,
    StringBuilder* result_builder) const {
  ASSERT(length != 0);
  result_builder->AddCharacter(decimal_digits[0]);
  if (length != 1) {
    result_builder->AddCharacter('.');
    result_builder->AddSubstring(&decimal_digits[1], length-1);
  }
  result_builder->AddCharacter(exponent_character_);
  if (exponent < 0) {
    result_builder->AddCharacter('-');
    exponent = -exponent;
  } else {
    if ((flags_ & EmitPositiveExponentSign) != 0) {
      result_builder->AddCharacter('+');
    }
  }
  if (exponent == 0) {
    result_builder->AddCharacter('0');
    return;
  }
  ASSERT(exponent < 1e4);
  const int MaxExponentLength = 5;
  char buffer[MaxExponentLength + 1];
  int first_char_pos = MaxExponentLength;
  buffer[first_char_pos] = '\0';
  while (exponent > 0) {
    buffer[--first_char_pos] = '0' + (exponent % 10);
    exponent /= 10;
  }
  result_builder->AddSubstring(&buffer[first_char_pos],
                               MaxExponentLength - first_char_pos);
}


void DoubleToStringConverter::CreateDecimalRepresentation(
    const char* decimal_digits,
    int length,
    int decimal_point,
    int digits_after_point,
    StringBuilder* result_builder) const {
  // Create a representation that is padded with zeros if needed.
  if (decimal_point <= 0) {
    // "0.00000decimal_rep".
    result_builder->AddCharacter('0');
    if (digits_after_point > 0) {
      result_builder->AddCharacter('.');
      result_builder->AddPadding('0', -decimal_point);
      ASSERT(length <= digits_after_point - (-decimal_point));
      result_builder->AddSubstring(decimal_digits, length);
      int remaining_digits = digits_after_point - (-decimal_point) - length;
      result_builder->AddPadding('0', remaining_digits);
    }
  } else if (decimal_point >= length) {
    // "decimal_rep0000.00000" or "decimal_rep.0000"
    result_builder->AddSubstring(decimal_digits, length);
    result_builder->AddPadding('0', decimal_point - length);
    if (digits_after_point > 0) {
      result_builder->AddCharacter('.');
      result_builder->AddPadding('0', digits_after_point);
    }
  } else {
    // "decima.l_rep000"
    ASSERT(digits_after_point > 0);
    result_builder->AddSubstring(decimal_digits, decimal_point);
    result_builder->AddCharacter('.');
    ASSERT(length - decimal_point <= digits_after_point);
    result_builder->AddSubstring(&decimal_digits[decimal_point],
                                 length - decimal_point);
    int remaining_digits = digits_after_point - (length - decimal_point);
    result_builder->AddPadding('0', remaining_digits);
  }
  if (digits_after_point == 0) {
    if ((flags_ & EmitTrailingDecimalPoint) != 0) {
      result_builder->AddCharacter('.');
    }
    if ((flags_ & EmitTrailingZeroAfterPoint) != 0) {
      result_builder->AddCharacter('0');
    }
  }
}


bool DoubleToStringConverter::ToShortest(double value,
                                         StringBuilder* result_builder) const {
  if (Double(value).IsSpecial()) {
    return HandleSpecialValues(value, result_builder);
  }

  int decimal_point;
  bool sign;
  const int DecimalRepCapacity = Base10MaximalLength + 1;
  char decimal_rep[DecimalRepCapacity];
  int decimal_rep_length;

  DoubleToAscii(value, ShortestMode, 0, decimal_rep, DecimalRepCapacity,
                &sign, &decimal_rep_length, &decimal_point);

  bool unique_zero = (flags_ & UniqueZero) != 0;
  if (sign && (value != 0.0 || !unique_zero)) {
    result_builder->AddCharacter('-');
  }

  int exponent = decimal_point - 1;
  if ((decimal_in_shortest_low_ <= exponent) &&
      (exponent < decimal_in_shortest_high_)) {
    CreateDecimalRepresentation(decimal_rep, decimal_rep_length,
                                decimal_point,
                                max(0, decimal_rep_length - decimal_point),
                                result_builder);
  } else {
    CreateExponentialRepresentation(decimal_rep, decimal_rep_length, exponent,
                                    result_builder);
  }
  return true;
}


bool DoubleToStringConverter::ToFixed(double value,
                                      int requested_digits,
                                      StringBuilder* result_builder) const {
  ASSERT(MaxFixedDigitsBeforePoint == 60);
  const double FirstNonFixed = 1e60;

  if (Double(value).IsSpecial()) {
    return HandleSpecialValues(value, result_builder);
  }

  if (requested_digits > MaxFixedDigitsAfterPoint) return false;
  if (value >= FirstNonFixed || value <= -FirstNonFixed) return false;

  // Find a sufficiently precise decimal representation of n.
  int decimal_point;
  bool sign;
  // Add space for the '\0' byte.
  const int DecimalRepCapacity = MaxFixedDigitsBeforePoint + MaxFixedDigitsAfterPoint + 1;
  char decimal_rep[DecimalRepCapacity];
  int decimal_rep_length;
  DoubleToAscii(value, FixedMode, requested_digits,
                decimal_rep, DecimalRepCapacity,
                &sign, &decimal_rep_length, &decimal_point);

  bool unique_zero = ((flags_ & UniqueZero) != 0);
  if (sign && (value != 0.0 || !unique_zero)) {
    result_builder->AddCharacter('-');
  }

  CreateDecimalRepresentation(decimal_rep, decimal_rep_length, decimal_point,
                              requested_digits, result_builder);
  return true;
}


bool DoubleToStringConverter::ToExponential(
    double value,
    int requested_digits,
    StringBuilder* result_builder) const {
  if (Double(value).IsSpecial()) {
    return HandleSpecialValues(value, result_builder);
  }

  if (requested_digits < -1) return false;
  if (requested_digits > MaxExponentialDigits) return false;

  int decimal_point;
  bool sign;
  // Add space for digit before the decimal point and the '\0' character.
  const int DecimalRepCapacity = MaxExponentialDigits + 2;
  ASSERT(DecimalRepCapacity > Base10MaximalLength);
  char decimal_rep[DecimalRepCapacity];
  int decimal_rep_length;

  if (requested_digits == -1) {
    DoubleToAscii(value, ShortestMode, 0,
                  decimal_rep, DecimalRepCapacity,
                  &sign, &decimal_rep_length, &decimal_point);
  } else {
    DoubleToAscii(value, PrecisionMode, requested_digits + 1,
                  decimal_rep, DecimalRepCapacity,
                  &sign, &decimal_rep_length, &decimal_point);
    ASSERT(decimal_rep_length <= requested_digits + 1);

    for (int i = decimal_rep_length; i < requested_digits + 1; ++i) {
      decimal_rep[i] = '0';
    }
    decimal_rep_length = requested_digits + 1;
  }

  bool unique_zero = ((flags_ & UniqueZero) != 0);
  if (sign && (value != 0.0 || !unique_zero)) {
    result_builder->AddCharacter('-');
  }

  int exponent = decimal_point - 1;
  CreateExponentialRepresentation(decimal_rep,
                                  decimal_rep_length,
                                  exponent,
                                  result_builder);
  return true;
}


bool DoubleToStringConverter::ToPrecision(
    double value, int precision, StringBuilder* result_builder) const {
  if (Double(value).IsSpecial()) {
    return HandleSpecialValues(value, result_builder);
  }

  if (precision < MinPrecisionDigits || precision > MaxPrecisionDigits) {
    return false;
  }

  // Find a sufficiently precise decimal representation of n.
  int decimal_point;
  bool sign;
  // Add one for the terminating null character.
  const int DecimalRepCapacity = MaxPrecisionDigits + 1;
  char decimal_rep[DecimalRepCapacity];
  int decimal_rep_length;

  DoubleToAscii(value, PrecisionMode, precision,
                decimal_rep, DecimalRepCapacity,
                &sign, &decimal_rep_length, &decimal_point);
  ASSERT(decimal_rep_length <= precision);

  bool unique_zero = ((flags_ & UniqueZero) != 0);
  if (sign && (value != 0.0 || !unique_zero)) {
    result_builder->AddCharacter('-');
  }

  // The exponent if we print the number as x.xxeyyy. That is with the
  // decimal point after the first digit.
  int exponent = decimal_point - 1;

  int extra_zero = ((flags_ & EmitTrailingZeroAfterPoint) != 0) ? 1 : 0;
  if ((-decimal_point + 1 > max_leading_padding_zeroes_in_precision_mode_) ||
      (decimal_point - precision + extra_zero >
  max_trailing_padding_zeroes_in_precision_mode_)) {
    // Fill buffer to contain 'precision' digits.
    // Usually the buffer is already at the correct length, but 'DoubleToAscii'
    // is allowed to return less characters.
    for (int i = decimal_rep_length; i < precision; ++i) {
      decimal_rep[i] = '0';
    }

    CreateExponentialRepresentation(decimal_rep,
                                    precision,
                                    exponent,
                                    result_builder);
  } else {
    CreateDecimalRepresentation(decimal_rep, decimal_rep_length, decimal_point,
                                max(0, precision - decimal_point),
                                result_builder);
  }
  return true;
}


static BignumDtoaMode DtoaToBignumDtoaMode(
    DoubleToStringConverter::DtoaMode dtoa_mode) {
  switch (dtoa_mode) {
    case DoubleToStringConverter::ShortestMode:  return BigNumDtoaModeShortest;
    case DoubleToStringConverter::FixedMode:     return BigNumDtoaModeFixed;
    case DoubleToStringConverter::PrecisionMode: return BigNumDtoaModePrecision;
  }
  UNREACHABLE(return BigNumDtoaModeShortest);
}


void DoubleToStringConverter::DoubleToAscii(double v,
                                            DtoaMode mode,
                                            int requested_digits,
                                            char* buffer,
                                            int buffer_length,
                                            bool* sign,
                                            int* length,
                                            int* point) {
  Vector<char> vector(buffer, buffer_length);
  ASSERT(!Double(v).IsSpecial());
  ASSERT(mode == ShortestMode || requested_digits >= 0);

  if (Double(v).Sign() < 0) {
    *sign = true;
    v = -v;
  } else {
    *sign = false;
  }

  if (mode == PrecisionMode && requested_digits == 0) {
    vector[0] = '\0';
    *length = 0;
    return;
  }

  if (v == 0) {
    vector[0] = '0';
    vector[1] = '\0';
    *length = 1;
    *point = 1;
    return;
  }

  bool fast_worked;
  switch (mode) {
    case ShortestMode:
      fast_worked = FastDtoa(v, FastDtoaModeShortest, 0, vector, length, point);
      break;
    case FixedMode:
      fast_worked = FastFixedDtoa(v, requested_digits, vector, length, point);
      break;
    case PrecisionMode:
      fast_worked = FastDtoa(v, FastDtoaModePrecision, requested_digits,
                             vector, length, point);
      break;
    default:
      UNREACHABLE(fast_worked = false);
  }
  if (fast_worked) 
    return;

  // If the fast dtoa didn't succeed use the slower bignum version.
  BignumDtoaMode bignum_mode = DtoaToBignumDtoaMode(mode);
  BignumDtoa(v, bignum_mode, requested_digits, vector, length, point);
  vector[*length] = '\0';
}


// Maximum number of significant digits in decimal representation.
// The longest possible double in decimal representation is
// (2^53 - 1) * 2 ^ -1074 that is (2 ^ 53 - 1) * 5 ^ 1074 / 10 ^ 1074
// (768 digits). If we parse a number whose first digits are equal to a
// mean of 2 adjacent doubles (that could have up to 769 digits) the result
// must be rounded to the bigger one unless the tail consists of zeros, so
// we don't need to preserve all the digits.
const int MaxSignificantDigits = 772;


static double SignedZero(bool sign) {
  return sign ? -0.0 : 0.0;
}


bool StringToDoubleConverter::StringToDouble(const char*& ptr, const char* end,
                                             double* out_value) {
  const char* start = ptr;

  // To make sure that iterator dereferencing is valid the following
  // convention is used:
  // 1. Each '++ptr' statement is followed by check for equality to 'end'.
  // 3. If 'ptr' becomes equal to 'end' the function returns or goes to
  // 'parsing_done'.
  // 4. 'ptr' is not dereferenced after the 'parsing_done' label.
  // 5. Code before 'parsing_done' may rely on 'ptr != end'.
  if (ptr >= end) return false;

  // The longest form of simplified number is: "-<significant digits>.1eXXX\0".
  const int BufferSize = MaxSignificantDigits + 10;
  char buffer[BufferSize];
  int buffer_pos = 0;

  // Exponent will be adjusted if insignificant digits of the integer part
  // or insignificant leading zeros of the fractional part are dropped.
  int exponent = 0;
  int significant_digits = 0;
  int insignificant_digits = 0;
  bool nonzero_digit_dropped = false;
  bool sign = false;

  if (*ptr == '+' || *ptr == '-') {
    sign = (*ptr == '-');
    ++ptr;
    if (ptr == end) {
      ptr = start;
      return false;
    }
  }

  bool leading_zero = false;
  if (*ptr == '0') {
    ++ptr;
    if (ptr == end) {
      *out_value = SignedZero(sign);
      return true;
    }

    leading_zero = true;

    // Ignore leading zeros in the integer part.
    while (*ptr == '0') {
      ++ptr;
      if (ptr == end) {
        *out_value = SignedZero(sign);
        return true;
      }
    }
  }

  // Copy significant digits of the integer part (if any) to the buffer.
  while (*ptr >= '0' && *ptr <= '9') {
    if (significant_digits < MaxSignificantDigits) {
      ASSERT(buffer_pos < BufferSize);
      buffer[buffer_pos++] = static_cast<char>(*ptr);
      significant_digits++;
    } else {
      insignificant_digits++;  // Move the digit into the exponential part.
      nonzero_digit_dropped = nonzero_digit_dropped || *ptr != '0';
    }
    ++ptr;
    if (ptr == end) goto parsing_done;
  }

  if (*ptr == '.') {
    ++ptr;
    if (ptr == end) {
      if (significant_digits == 0 && !leading_zero) {
        ptr = start;
        return false;
      } else {
        goto parsing_done;
      }
    }

    if (significant_digits == 0) {
      // Integer part consists of 0 or is absent. Significant digits start after
      // leading zeros (if any).
      while (*ptr == '0') {
        ++ptr;
        if (ptr == end) {
          *out_value = SignedZero(sign);
          return true;
        }
        exponent--;  // Move this 0 into the exponent.
      }
    }

    // There is a fractional part.
    while (*ptr >= '0' && *ptr <= '9') {
      if (significant_digits < MaxSignificantDigits) {
        ASSERT(buffer_pos < BufferSize);
        buffer[buffer_pos++] = static_cast<char>(*ptr);
        significant_digits++;
        exponent--;
      } else {
        // Ignore insignificant digits in the fractional part.
        nonzero_digit_dropped = nonzero_digit_dropped || *ptr != '0';
      }
      ++ptr;
      if (ptr == end) goto parsing_done;
    }
  }

  if (!leading_zero && exponent == 0 && significant_digits == 0) {
    // If leading_zeros is true then the string contains zeros.
    // If exponent < 0 then string was [+-]\.0*...
    // If significant_digits != 0 the string is not equal to 0.
    // Otherwise there are no digits in the string.
    ptr = start;
    return false;
  }

  // Parse exponential part.
  if (*ptr == 'e' || *ptr == 'E') {
    ++ptr;
    if (ptr == end) {
      --ptr;
      goto parsing_done;
    }
    char sign = 0;
    if (*ptr == '+' || *ptr == '-') {
      sign = static_cast<char>(*ptr);
      ++ptr;
      if (ptr == end) {
        ptr -= 2;
        goto parsing_done;
      }
    }

    if (*ptr < '0' || *ptr > '9') {
      if (sign)
        --ptr;
      --ptr;
      goto parsing_done;
    }

    const int max_exponent = INT_MAX / 2;
    ASSERT(-max_exponent / 2 <= exponent && exponent <= max_exponent / 2);
    int num = 0;
    do {
      // Check overflow.
      int digit = *ptr - '0';
      if (num >= max_exponent / 10
          && !(num == max_exponent / 10 && digit <= max_exponent % 10)) {
        num = max_exponent;
      } else {
        num = num * 10 + digit;
      }
      ++ptr;
    } while (ptr != end && *ptr >= '0' && *ptr <= '9');

    exponent += (sign == '-' ? -num : num);
  }

parsing_done:
  exponent += insignificant_digits;

  if (nonzero_digit_dropped) {
    buffer[buffer_pos++] = '1';
    exponent--;
  }

  ASSERT(buffer_pos < BufferSize);
  buffer[buffer_pos] = '\0';

  double converted = Strtod(Vector<const char>(buffer, buffer_pos), exponent);
  *out_value = sign ? -converted : converted;
  return true;
}

} // namespace dtoa

} // namespace stp
