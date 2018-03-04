// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/TextWriter.h"

#include "Base/Text/AsciiChar.h"
#include "Base/Text/TextEncoding.h"
#include "Base/Text/Utf.h"

namespace stp {

/**
 * @class TextWriter
 * Locale-independent replacement for std::ostream.
 * Useful for writing text-based representation of data, for example XML and JSON.
 * it should NOT be used to build text displayed to the user since it does not
 * support locale (and will not do; on purpose).
 */

void TextWriter::onEndLine() {
  onWriteChar('\n');
}

void TextWriter::onFlush() {}

bool TextWriter::isConsoleWriter() const {
  return false;
}

void TextWriter::writeRune(char32_t rune) {
  ASSERT(unicode::IsValidRune(rune));
  if (isAscii(rune)) {
    onWriteChar(static_cast<char>(rune));
  } else {
    onWriteRune(rune);
  }
}

void TextWriter::onIndent(int count, char c) {
  constexpr StringSpan SpacePadding = "                    ";
  constexpr int ChunkSize = SpacePadding.length();
  char custom_padding[ChunkSize];

  if (count <= 1) {
    if (count == 1) {
      onWriteChar(c);
    }
    return;
  }

  const char* templ;
  if (c == ' ') {
    templ = SpacePadding.data();
  } else {
    memset(custom_padding, c, ChunkSize);
    templ = custom_padding;
  }

  while (count > 0) {
    int chunk_size = min(count, ChunkSize);
    onWriteString(StringSpan(templ, chunk_size));
    count -= chunk_size;
  }
}

void TextWriter::onWriteChar(char c) {
  onWriteString(StringSpan(&c, 1));
}

void TextWriter::onWriteRune(char32_t rune) {
  char units[Utf8::MaxEncodedRuneLength];
  int n = EncodeUtf(units, rune);
  onWriteString(StringSpan(units, n));
}

#if ASSERT_IS_ON
bool TextWriter::isValidChar(char c) {
  return isAscii(c);
}
#endif

namespace {

class NullTextWriter final : public TextWriter {
 public:
  TextEncoding getEncoding() const { return TextEncoding(); }
  void onWriteChar(char c) override {}
  void onWriteRune(char32_t c) override {}
  void onWriteString(StringSpan text) override {}
  void onIndent(int count, char c) override {}
};

} // namespace

TextWriter& TextWriter::nullWriter() {
  static AlignedStorage<NullTextWriter> g_storage;
  return *new(g_storage.bytes) NullTextWriter();
}

} // namespace stp
