// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/BitArray.h"

#include "Base/Error/BasicExceptions.h"
#include "Base/Io/TextWriter.h"
#include "Base/Text/AsciiChar.h"

namespace stp {
namespace detail {

void formatBitArrayAsBinary(TextWriter& writer, const uintptr_t* words, int size) {
  constexpr int BitCountInWord = 8 * sizeof(uintptr_t);
  const int WordCount = (size + BitCountInWord - 1) / BitCountInWord;

  int valid_bit_count = size % BitCountInWord;
  if (valid_bit_count == 0)
    valid_bit_count = BitCountInWord;

  char bit_chars[BitCountInWord];
  for (int i = WordCount - 1; i >= 0; --i) {
    uintptr_t word = words[i];

    char* end = bit_chars + BitCountInWord;
    char* begin = end;
    for (int j = 0; j < valid_bit_count; ++j) {
      *--begin = (word & 1) + '0';
      word >>= 1;
    }
    writer << StringSpan(begin, end - begin);

    valid_bit_count = BitCountInWord;
  }
}

static void formatBitArrayAsHex(TextWriter& writer, const uintptr_t* words, int size) {
  constexpr int BitCountInWord = 8 * sizeof(uintptr_t);
  const int WordCount = (size + BitCountInWord - 1) / BitCountInWord;

  int valid_bit_count = size % BitCountInWord;
  if (valid_bit_count == 0)
    valid_bit_count = BitCountInWord;

  char bit_chars[BitCountInWord];
  for (int i = WordCount - 1; i >= 0; --i) {
    uintptr_t word = words[i];

    char* end = bit_chars + BitCountInWord;
    char* begin = end;
    for (int j = 0; j < valid_bit_count; j += 4) {
      *--begin = nibbleToHexDigitUpper(word & 0xF);
      word >>= 4;
    }
    writer << StringSpan(begin, end - begin);

    valid_bit_count = BitCountInWord;
  }
}

void formatBitArray(TextWriter& out, const StringSpan& opts, const uintptr_t* words, int size) {
  enum class Format { Binary, Hexadecimal };
  Format format = Format::Binary;

  if (!opts.isEmpty()) {
    bool ok = opts.length() == 1;
    if (toUpperAscii(opts[0]) == 'X') {
      format = Format::Hexadecimal;
    } else {
      ok = toUpperAscii(opts[0]) == 'B';
    }
    PANIC_IF(!ok, "invalid format for BitArray");
  }

  switch (format) {
    case Format::Binary:
      formatBitArrayAsBinary(out, words, size);
      break;
    case Format::Hexadecimal:
      formatBitArrayAsHex(out, words, size);
      break;
  }
}

} // namespace detail
} // namespace stp
