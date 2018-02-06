// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/TextWriter.h"

#include "Base/Dtoa/Dtoa.h"
#include "Base/Text/TextEncoding.h"

namespace stp {

const EndOfLineTag EndOfLine;

void TextWriter::OnEndLine() {
  Write('\n');
}

void TextWriter::OnFlush() {}

bool TextWriter::IsConsoleWriter() const {
  return false;
}

void TextWriter::OnIndent(int count, char c) {
  ASSERT(count >= 0);

  constexpr StringSpan SpacePadding = "                    ";
  constexpr int ChunkSize = SpacePadding.size();
  char custom_padding[ChunkSize];

  if (count <= 1) {
    if (count == 1)
      Write(c);
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
    WriteAscii(StringSpan(templ, chunk_size));
    count -= chunk_size;
  }
}

namespace {

class NullTextWriter final : public TextWriter {
 public:
  TextEncoding GetEncoding() const { return TextEncoding::BuiltinUtf8(); }
  void OnWriteAsciiChar(char c) override {}
  void OnWriteUnicodeChar(char32_t c) override {}
  void OnWriteAscii(StringSpan text) override {}
  void OnWriteUtf8(StringSpan text) override {}
  void OnWriteUtf16(String16Span text) override {}
  void OnIndent(int count, char c) override {}
};

} // namespace

TextWriter& TextWriter::Null() {
  static AlignedStorage<NullTextWriter> g_storage;
  return *new(g_storage.bytes) NullTextWriter();
}

} // namespace stp
