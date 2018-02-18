// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/BufferSpan.h"

#include "Base/Error/BasicExceptions.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Type/Formattable.h"

namespace stp {

bool tryParse(StringSpan input, MutableBufferSpan output) {
  if (input.size() & 1)
    return false;

  int num_to_output = input.size() >> 1;
  if (num_to_output != output.size())
    return false;

  auto* output_bytes = static_cast<byte_t*>(output.data());
  for (int i = 0; i < num_to_output; ++i) {
    char mc = input[i * 2 + 0];
    char lc = input[i * 2 + 1];

    int msb = tryParseHexDigit(mc);
    int lsb = tryParseHexDigit(lc);
    if (msb < 0 || lsb < 0)
      return false;

    output_bytes[i] = static_cast<byte_t>((msb << 4) | lsb);
  }
  return true;
}

static constexpr char UpperHexChars[] = "0123456789ABCDEF";
static constexpr char LowerHexChars[] = "0123456789abcdef";

static void formatBufferSimple(TextWriter& out, const void* data, int size, bool uppercase) {
  char out_buffer[256];
  auto* bytes = static_cast<const byte_t*>(data);
  while (size > 0) {
    int bytes_to_print = min(size, isizeof(out_buffer) / 2);
    int chars_to_print = bytes_to_print * 2;

    formatBuffer(MutableStringSpan(out_buffer, chars_to_print), bytes, bytes_to_print, uppercase);
    out << StringSpan(out_buffer, chars_to_print);
    bytes += bytes_to_print;
    size -= bytes_to_print;
  }
}

void formatBuffer(TextWriter& out, const void* data, int size) {
  formatBufferSimple(out, data, size, true);
}

static void formatByte(TextWriter& out, byte_t b) {
  char string[3];
  string[0] = nibbleToHexDigitUpper((b >> 4) & 0x0F);
  string[1] = nibbleToHexDigitUpper((b >> 0) & 0x0F);
  string[2] = '\0';
  out << string;
}

void formatBuffer(TextWriter& out, const void* data, int size, const StringSpan& opts) {
  enum class Mode { Simple, MemoryDump };

  Mode mode = Mode::Simple;
  bool uppercase = true;

  for (int i = 0; i < opts.size(); ++i) {
    char c = opts[i];
    switch (c) {
      case 'x':
      case 'X':
        uppercase = isUpperAscii(c);
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
    formatBufferSimple(out, data, size, uppercase);
    return;
  }

  constexpr int BytesPerLine = 16;
  auto* bytes = static_cast<const byte_t*>(data);
  while (size > 0) {
    out << bytes;

    out << ' ';
    for (int i = 0; i < BytesPerLine; ++i) {
      if (i < size) {
        formatByte(out, bytes[i]);
      } else {
        out << "  ";
      }
      out << ' ';
    }
    for (int i = 0; i < BytesPerLine; ++i) {
      if (i >= size)
        break;
      char c = static_cast<char>(bytes[i]);
      if (!isPrintAscii(c))
        c = '.';
      out << c;
    }
    out << '\n';

    bytes += BytesPerLine;
    size -= BytesPerLine;
  }
}

void formatBuffer(MutableStringSpan out, const void* data, int size, bool uppercase) {
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
