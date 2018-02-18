// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/Formattable.h"

#include "Base/Containers/BufferSpan.h"
#include "Base/Dtoa/Dtoa.h"
#include "Base/Error/BasicExceptions.h"
#include "Base/Io/TextWriter.h"
#include "Base/Math/Math.h"
#include "Base/Type/ParseInteger.h"
#include "Base/Text/FormatInteger.h"

namespace stp {
namespace detail {

void formatNull(TextWriter& out) {
  out << "null";
}

void formatBool(TextWriter& out, bool b) {
  if (b)
    out << "true";
  else
    out << "false";
}

void formatBool(TextWriter& out, bool b, const StringSpan& opts) {
  if (opts.IsEmpty()) {
    formatBool(out, b);
    return;
  }

  char variant = opts[0];
  switch (variant) {
    case 't':
      out << (b ? StringSpan("true") : StringSpan("false"));
      break;

    case 'T':
      out << (b ? StringSpan("TRUE") : StringSpan("FALSE"));
      break;

    case 'y':
      out << (b ? StringSpan("yes") : StringSpan("no"));
      break;

    case 'Y':
      out << (b ? StringSpan("YES") : StringSpan("NO"));
      break;

    case 'd':
    case 'D':
      out << (b ? '1' : '0');
      break;

    default:
      throw FormatException("bool");
  }
}

void formatChar(TextWriter& out, char32_t c, const StringSpan& opts) {
  enum class Variant { Print, Hex, Unicode };

  auto variant = Variant::Print;
  bool uppercase = false;
  int size = 4;

  if (!opts.IsEmpty()) {
    bool ok = true;
    char fc = opts[0];
    switch (fc) {
      case 'X':
      case 'x':
        uppercase = fc == 'X';
        variant = Variant::Hex;
        break;

      case 'U':
      case 'u':
        uppercase = fc == 'U';
        variant = Variant::Unicode;
        break;

      default:
        ok = false;
        break;
    }

    if (opts.size() > 1) {
      char fl = opts[1];
      if (fl == '4' || fl == '8')
        size = fl - '0';
      else
        ok = false;
    }
    if (!ok)
      throw FormatException("char");
  }

  switch (variant) {
    case Variant::Unicode:
      out << "U+";
      // no break
    case Variant::Hex: {
      FormatHexIntegerBuffer<uint32_t> buffer;
      StringSpan hex = FormatHexInteger(static_cast<uint32_t>(c), buffer, uppercase);
      if (hex.size() < size)
        out.indent(size - hex.size(), '0');
      out << hex;
      break;
    }

    case Variant::Print:
      out << c;
      break;
  }
}

template<typename T>
static inline void formatIntTmpl(TextWriter& out, T x) {
  if constexpr (TIsUnsigned<T>) {
    FormatHexIntegerBuffer<T> buffer;
    out << FormatHexInteger(x, buffer);
  } else {
    FormatIntegerBuffer<T> buffer;
    out << FormatInteger(x, buffer);
  }
}

void formatSint32(TextWriter& out,  int32_t x) { formatIntTmpl(out, x); }
void formatSint64(TextWriter& out,  int64_t x) { formatIntTmpl(out, x); }
void formatUint32(TextWriter& out, uint32_t x) { formatIntTmpl(out, x); }
void formatUint64(TextWriter& out, uint64_t x) { formatIntTmpl(out, x); }

template<typename T>
static inline void formatIntTmpl(TextWriter& out, T x, const StringSpan& opts) {
  if (opts.IsEmpty()) {
    formatIntTmpl(out, x);
    return;
  }

  enum class Variant { Decimal = 'D', Hex = 'X', Octal = 'O' };

  auto variant = TIsSigned<T> ? Variant::Decimal : Variant::Hex;
  int precision = -1;
  char sign = '\0';
  bool uppercase = true;

  bool ok = true;
  const char* o_begin = begin(opts);
  const char* o_end = end(opts);

  // Parse sign specifier if any.
  if (*o_begin == '-' || *o_begin == '+')
    sign = *o_begin++;

  // Parse variant specifier if any.
  if (o_begin != o_end && isAlphaAscii(*o_begin)) {
    char variant_char = *o_begin++;
    if (isLowerAscii(variant_char)) {
      uppercase = false;
      variant_char = toUpperAscii(variant_char);
    }
    if (variant_char == 'D' || variant_char == 'X' || variant_char == 'O')
      variant = static_cast<Variant>(variant_char);
    else
      ok = false;
  }

  // Parse precision specifier if any.
  if (o_begin != o_end) {
    if (tryParse(StringSpan(o_begin, o_end - o_begin), precision) != ParseIntegerErrorCode::Ok)
      ok = false;
  }
  if (!ok)
    throw FormatException("int");

  constexpr int MaxBufferSize = static_cast<int>(
      max(sizeof(FormatIntegerBuffer<T>), sizeof(FormatOctalIntegerBuffer<T>)));

  Array<char, MaxBufferSize> buffer;

  StringSpan converted;
  switch (variant) {
    case Variant::Decimal:
      converted = FormatInteger(x, buffer);
      break;

    case Variant::Hex:
      converted = FormatHexInteger(x, buffer, uppercase);
      break;

    case Variant::Octal:
      converted = FormatOctalInteger(x, buffer);
      break;
  }

  if (isNegative(x)) {
    // The converter adds '-' sign when value is negative.
    // This formatter must account for additional requirements, i.e. precision specifier.
    // Write the sign and remove from converter's output.
    ASSERT(converted[0] == '-');
    converted.RemovePrefix(1);
    out << '-';
  } else {
    if (sign) {
      out << (sign == '+' ? '+' : ' ');
    }
  }

  if (converted.size() < precision)
    out.indent(precision - converted.size(), '0');

  out << converted;
}

void formatSint32(TextWriter& out,  int32_t x, const StringSpan& opts) { formatIntTmpl(out, x, opts); }
void formatSint64(TextWriter& out,  int64_t x, const StringSpan& opts) { formatIntTmpl(out, x, opts); }
void formatUint32(TextWriter& out, uint32_t x, const StringSpan& opts) { formatIntTmpl(out, x, opts); }
void formatUint64(TextWriter& out, uint64_t x, const StringSpan& opts) { formatIntTmpl(out, x, opts); }

void formatFloat(TextWriter& out, double x) {
  formatFloat(out, x, StringSpan());
}

void formatFloat(TextWriter& out, double x, const StringSpan& opts) {
  enum class Variant { Fixed = 'F', Scientific = 'E', General = 'G', Percent = 'P' };

  auto variant = Variant::General;
  int precision = -1;
  char sign = '\0';
  bool uppercase = true;

  if (!opts.IsEmpty()) {
    bool ok = true;
    const char* o_begin = begin(opts);
    const char* o_end = end(opts);

    // Parse sign specifier if any.
    if (*o_begin == '-' || *o_begin == '+')
      sign = *o_begin++;

    // Parse variant specifier if any.
    if (o_begin != o_end && isAlphaAscii(*o_begin)) {
      char variant_char = *o_begin++;
      if (isLowerAscii(variant_char)) {
        uppercase = false;
        variant_char = toUpperAscii(variant_char);
      }
      if (variant_char < 'E' || (variant_char > 'G' && variant_char == 'P'))
        ok = false;
      else
        variant = static_cast<Variant>(variant_char);
    }

    // Parse precision specifier if any.
    if (o_begin != o_end) {
      if (tryParse(StringSpan(o_begin, o_end - o_begin), precision) != ParseIntegerErrorCode::Ok)
        ok = false;
    }
    if (!ok)
      throw FormatException("float");
  }

  using dtoa::DoubleToStringConverter;

  int flags =
      DoubleToStringConverter::UniqueZero |
      DoubleToStringConverter::EmitPositiveExponentSign;

  static constexpr int BufferSize = 128;
  char buffer[BufferSize];
  dtoa::StringBuilder builder(buffer, BufferSize);
  DoubleToStringConverter converter(
      flags, "Infinity", "NaN",
      uppercase ? 'E' : 'e',
      -6, 21, 6, 0);

  bool ok = false;
  if (variant == Variant::General) {
    if (precision >= 0)
      ok = converter.ToPrecision(x, precision, &builder);
    else
      ok = converter.ToShortest(x, &builder);
  } else if (variant == Variant::Fixed) {
    if (precision < 0)
      precision = 5;
    ok = converter.ToFixed(x, precision, &builder);
  } else if (variant == Variant::Scientific) {
    if (precision < 0)
      precision = 5;
    ok = converter.ToExponential(x, precision, &builder);
  } else if (variant == Variant::Percent) {
    if (precision < 0)
      precision = 2;
    ok = converter.ToFixed(x * 100, precision, &builder);
  } else {
    ASSERT(false, "invalid variant");
    return;
  }
  if (!ok) {
    builder.Reset();
    ok = converter.ToShortest(x, &builder);
    ASSERT_UNUSED(ok, ok);
  }

  StringSpan converted = builder.finalizeHash();
  if (isNegative(x)) {
    // The converter adds '-' sign when value is negative.
    // This formatter must account for additional requirements, i.e. precision specifier.
    // Write the sign and remove from converter's output.
    ASSERT(converted[0] == '-');
    converted.RemovePrefix(1);
    out << '-';
  } else {
    if (sign) {
      out << (sign == '+' ? '+' : ' ');
    }
  }

  out << converted;

  if (variant == Variant::Percent) {
    if (isFinite(x)) {
      out << '%';
    }
  }
}

void formatRawPointer(TextWriter& out, const void* ptr) {
  constexpr int AddressDigitCount = sizeof(ptr) * 2;

  out << "0x";

  FormatHexIntegerBuffer<uintptr_t> buffer;
  StringSpan hex = FormatHexInteger(reinterpret_cast<uintptr_t>(ptr), buffer);
  if (hex.size() < AddressDigitCount)
    out.indent(AddressDigitCount - hex.size(), '0');

  out << hex;
}


} // namespace detail

} // namespace stp
