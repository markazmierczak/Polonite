/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991, 2000, 2001 by Lucent Technologies.
 * Copyright (C) 2002, 2005, 2006, 2007, 2008, 2010, 2012 Apple Inc.
 * Copyright 2017 Polonite Authors.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR LUCENT MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/

#include "Base/Dtoa/Dtoa.h"

namespace stp {

static inline StringSpan FormatStringTruncatingTrailingZerosIfNeeded(
    FloatToStringBuffer buffer, dtoa::StringBuilder& builder) {
  int length = builder.position();

  // If there is an exponent, stripping trailing zeros would be incorrect.
  // FIXME: Zeros should be stripped before the 'e'.
  if (memchr(buffer, 'e', length))
    return builder.finalize();

  int decimal_point_position = 0;
  for (; decimal_point_position < length; ++decimal_point_position) {
    if (buffer[decimal_point_position] == '.')
      break;
  }

  // No decimal seperator found, early exit.
  if (decimal_point_position == length)
    return builder.finalize();

  int truncated_size = length - 1;
  for (; truncated_size > decimal_point_position; --truncated_size) {
    if (buffer[truncated_size] != '0')
      break;
  }

  // No trailing zeros found to strip.
  if (truncated_size == length - 1)
    return builder.finalize();

  // If we removed all trailing zeros, remove the decimal point as well.
  if (truncated_size == decimal_point_position) {
    ASSERT(truncated_size > 0);
    --truncated_size;
  }

  // Truncate the StringBuilder, and return the final result.
  builder.SetPosition(truncated_size + 1);
  return builder.finalize();
}

StringSpan FloatToFixedPrecisionString(
    FloatToStringBuffer buffer, double value, int precision) {
  // Mimic String::format("%.[precision]g", ...), but use dtoas rounding
  // facilities.
  // "g": Signed value printed in f or e format, whichever is more compact for
  // the given value and precision.
  // The e format is used only when the exponent of the value is less than -4 or
  // greater than or equal to the precision argument. Trailing zeros are
  // truncated, and the decimal point appears only if one or more digits follow
  // it.
  // "precision": The precision specifies the maximum number of significant
  // digits printed.
  dtoa::StringBuilder builder(buffer, FloatToStringBufferLength);
  const dtoa::DoubleToStringConverter& converter =
      dtoa::DoubleToStringConverter::EcmaScriptConverter();
  converter.ToPrecision(value, precision, &builder);
  // FIXME: Trailing zeros should never be added in the first place. The
  // current implementation does not strip when there is an exponent, eg.
  // 1.50000e+10.
  return FormatStringTruncatingTrailingZerosIfNeeded(buffer, builder);
}

} // namespace stp
