// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/TextWriter.h"

#include "Base/Dtoa/Dtoa.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/TextEncoding.h"
#include "Base/Text/Utf.h"

namespace stp {

void TextWriter::OnEndLine() {
  OnWriteChar('\n');
}

void TextWriter::OnFlush() {}

bool TextWriter::isConsoleWriter() const {
  return false;
}

void TextWriter::WriteRune(char32_t rune) {
  ASSERT(unicode::IsValidRune(rune));
  if (isAscii(rune)) {
    OnWriteChar(static_cast<char>(rune));
  } else {
    OnWriteRune(rune);
  }
}

void TextWriter::OnIndent(int count, char c) {
  constexpr StringSpan SpacePadding = "                    ";
  constexpr int ChunkSize = SpacePadding.size();
  char custom_padding[ChunkSize];

  if (count <= 1) {
    if (count == 1) {
      OnWriteChar(c);
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
    int chunk_size = Min(count, ChunkSize);
    OnWriteString(StringSpan(templ, chunk_size));
    count -= chunk_size;
  }
}

void TextWriter::OnWriteChar(char c) {
  OnWriteString(StringSpan(&c, 1));
}

void TextWriter::OnWriteRune(char32_t rune) {
  char units[Utf8::MaxEncodedRuneLength];
  int n = EncodeUtf(units, rune);
  OnWriteString(StringSpan(units, n));
}

#if ASSERT_IS_ON
bool TextWriter::IsValidChar(char c) {
  return isAscii(c);
}
#endif

namespace {

class NullTextWriter final : public TextWriter {
 public:
  TextEncoding GetEncoding() const { return TextEncoding(); }
  void OnWriteChar(char c) override {}
  void OnWriteRune(char32_t c) override {}
  void OnWriteString(StringSpan text) override {}
  void OnIndent(int count, char c) override {}
};

} // namespace

TextWriter& TextWriter::Null() {
  static AlignedStorage<NullTextWriter> g_storage;
  return *new(g_storage.bytes) NullTextWriter();
}

} // namespace stp
