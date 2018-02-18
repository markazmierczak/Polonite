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

void FormatNull(TextWriter& out) {
  out << "null";
}

void FormatBool(TextWriter& out, bool b) {
  if (b)
    out << "true";
  else
    out << "false";
}

void FormatBool(TextWriter& out, bool b, const StringSpan& opts) {
  if (opts.IsEmpty()) {
    FormatBool(out, b);
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

void FormatChar(TextWriter& out, char32_t c, const StringSpan& opts) {
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
        out.Indent(size - hex.size(), '0');
      out << hex;
      break;
    }

    case Variant::Print:
      out << c;
      break;
  }
}

template<typename T>
static inline void FormatIntTmpl(TextWriter& out, T x) {
  if constexpr (TIsUnsigned<T>) {
    FormatHexIntegerBuffer<T> buffer;
    out << FormatHexInteger(x, buffer);
  } else {
    FormatIntegerBuffer<T> buffer;
    out << FormatInteger(x, buffer);
  }
}

void FormatSInt32(TextWriter& out,  int32_t x) { FormatIntTmpl(out, x); }
void FormatSInt64(TextWriter& out,  int64_t x) { FormatIntTmpl(out, x); }
void FormatUInt32(TextWriter& out, uint32_t x) { FormatIntTmpl(out, x); }
void FormatUInt64(TextWriter& out, uint64_t x) { FormatIntTmpl(out, x); }

template<typename T>
static inline void FormatIntTmpl(TextWriter& out, T x, const StringSpan& opts) {
  if (opts.IsEmpty()) {
    FormatIntTmpl(out, x);
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
  if (o_begin != o_end && IsAlphaAscii(*o_begin)) {
    char variant_char = *o_begin++;
    if (IsLowerAscii(variant_char)) {
      uppercase = false;
      variant_char = ToUpperAscii(variant_char);
    }
    if (variant_char == 'D' || variant_char == 'X' || variant_char == 'O')
      variant = static_cast<Variant>(variant_char);
    else
      ok = false;
  }

  // Parse precision specifier if any.
  if (o_begin != o_end) {
    if (TryParse(StringSpan(o_begin, o_end - o_begin), precision) != ParseIntegerErrorCode::Ok)
      ok = false;
  }
  if (!ok)
    throw FormatException("int");

  constexpr int MaxBufferSize = static_cast<int>(
      Max(sizeof(FormatIntegerBuffer<T>), sizeof(FormatOctalIntegerBuffer<T>)));

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

  if (IsNegative(x)) {
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
    out.Indent(precision - converted.size(), '0');

  out << converted;
}

void FormatSInt32(TextWriter& out,  int32_t x, const StringSpan& opts) { FormatIntTmpl(out, x, opts); }
void FormatSInt64(TextWriter& out,  int64_t x, const StringSpan& opts) { FormatIntTmpl(out, x, opts); }
void FormatUInt32(TextWriter& out, uint32_t x, const StringSpan& opts) { FormatIntTmpl(out, x, opts); }
void FormatUInt64(TextWriter& out, uint64_t x, const StringSpan& opts) { FormatIntTmpl(out, x, opts); }

void FormatFloat(TextWriter& out, double x) {
  FormatFloat(out, x, StringSpan());
}

void FormatFloat(TextWriter& out, double x, const StringSpan& opts) {
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
    if (o_begin != o_end && IsAlphaAscii(*o_begin)) {
      char variant_char = *o_begin++;
      if (IsLowerAscii(variant_char)) {
        uppercase = false;
        variant_char = ToUpperAscii(variant_char);
      }
      if (variant_char < 'E' || (variant_char > 'G' && variant_char == 'P'))
        ok = false;
      else
        variant = static_cast<Variant>(variant_char);
    }

    // Parse precision specifier if any.
    if (o_begin != o_end) {
      if (TryParse(StringSpan(o_begin, o_end - o_begin), precision) != ParseIntegerErrorCode::Ok)
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

  StringSpan converted = builder.Finalize();
  if (IsNegative(x)) {
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
    if (IsFinite(x)) {
      out << '%';
    }
  }
}

void FormatRawPointer(TextWriter& out, const void* ptr) {
  constexpr int AddressDigitCount = sizeof(ptr) * 2;

  out << "0x";

  FormatHexIntegerBuffer<uintptr_t> buffer;
  StringSpan hex = FormatHexInteger(reinterpret_cast<uintptr_t>(ptr), buffer);
  if (hex.size() < AddressDigitCount)
    out.Indent(AddressDigitCount - hex.size(), '0');

  out << hex;
}


void FormatContiguousGenericExt(
    TextWriter& out,
    const void* data, int size, int item_size,
    const StringSpan& opts,
    void (*item_format)(TextWriter& out, const void* item, const StringSpan& opts)) {
  out << '[';
  for (int i = 0; i < size; ++i) {
    if (i != 0)
      out << ", ";
    item_format(out, static_cast<const byte_t*>(data) + i * item_size, opts);
  }
  out << ']';
}

} // namespace detail

static constexpr char UpperHexChars[] = "0123456789ABCDEF";
static constexpr char LowerHexChars[] = "0123456789abcdef";

static void FormatBufferSimple(TextWriter& out, const void* data, int size, bool uppercase) {
  char out_buffer[256];
  auto* bytes = static_cast<const byte_t*>(data);
  while (size > 0) {
    int bytes_to_print = Min(size, isizeof(out_buffer) / 2);
    int chars_to_print = bytes_to_print * 2;

    FormatBuffer(MutableStringSpan(out_buffer, chars_to_print), bytes, bytes_to_print, uppercase);
    out << StringSpan(out_buffer, chars_to_print);
    bytes += bytes_to_print;
    size -= bytes_to_print;
  }
}

void FormatBuffer(TextWriter& out, const void* data, int size) {
  FormatBufferSimple(out, data, size, true);
}

static void FormatByte(TextWriter& out, byte_t b) {
  char string[3];
  string[0] = NibbleToHexDigitUpper((b >> 4) & 0x0F);
  string[1] = NibbleToHexDigitUpper((b >> 0) & 0x0F);
  string[2] = '\0';
  out << string;
}

void FormatBuffer(TextWriter& out, const void* data, int size, const StringSpan& opts) {
  enum class Mode { Simple, MemoryDump };

  Mode mode = Mode::Simple;
  bool uppercase = true;

  for (int i = 0; i < opts.size(); ++i) {
    char c = opts[i];
    switch (c) {
      case 'x':
      case 'X':
        uppercase = IsUpperAscii(c);
        break;
      case 'd':
      case 'D':
        mode = Mode::MemoryDump;
        break;
      default:
        throw FormatException("Buffer");
    }
  }

  if (mode == Mode::Simple) {
    FormatBufferSimple(out, data, size, uppercase);
    return;
  }

  constexpr int BytesPerLine = 16;
  auto* bytes = static_cast<const byte_t*>(data);
  while (size > 0) {
    out << bytes;

    out << ' ';
    for (int i = 0; i < BytesPerLine; ++i) {
      if (i < size) {
        FormatByte(out, bytes[i]);
      } else {
        out << "  ";
      }
      out << ' ';
    }
    for (int i = 0; i < BytesPerLine; ++i) {
      if (i >= size)
        break;
      char c = static_cast<char>(bytes[i]);
      if (!IsPrintAscii(c))
        c = '.';
      out << c;
    }
    out << '\n';

    bytes += BytesPerLine;
    size -= BytesPerLine;
  }
}

void FormatBuffer(MutableStringSpan out, const void* data, int size, bool uppercase) {
  ASSERT(out.size() >= size * 2);

  const char* hex_chars = uppercase ? UpperHexChars : LowerHexChars;

  auto* bytes = static_cast<const byte_t*>(data);
  for (int i = 0; i < size; ++i) {
    byte_t b = bytes[i];
    out[i * 2 + 0] = hex_chars[(b >> 4) & 0xF];
    out[i * 2 + 1] = hex_chars[(b >> 0) & 0xF];
  }
}

} // namespace stp
