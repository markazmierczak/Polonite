// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_TEXTWRITER_H_
#define STP_BASE_IO_TEXTWRITER_H_

#include "Base/Containers/BufferSpanFwd.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/FormatFwd.h"
#include "Base/Text/StringSpan.h"
#include "Base/Text/Utf.h"

namespace stp {

class TextEncoding;

struct EndOfLineTag {};
BASE_EXPORT extern const EndOfLineTag EndOfLine;

// Locale-independent replacement for std::ostream.
// Useful for writing text-based representation of data, for example XML and JSON.
// it should NOT be used to build text displayed to the user since it does not
// support locale (and will not do; on purpose).
class BASE_EXPORT TextWriter {
 public:
  static TextWriter& Null();

  TextWriter() {}
  virtual ~TextWriter() {}

  void Write(char c);
  void Write(char16_t c) { Write(static_cast<char32_t>(c)); }
  void Write(wchar_t c) { Write(char_cast<char32_t>(c)); }
  void Write(char32_t c);

  void WriteAscii(StringSpan text);

  void Write(StringSpan text) { OnWriteUtf8(text); }
  void Write(String16Span text) { OnWriteUtf16(text); }
  void Write(const BufferSpan& text, TextEncoding encoding);

  void Indent(int count, char c = ' ');

  void Flush() { OnFlush(); }

  virtual TextEncoding GetEncoding() const = 0;

  // Simple RTTI:
  virtual bool IsConsoleWriter() const;

  friend TextWriter& operator<<(TextWriter& out, EndOfLineTag) {
    out.OnEndLine();
    return out;
  }

 protected:
  virtual void OnWriteAsciiChar(char c) = 0;
  virtual void OnWriteUnicodeChar(char32_t c) = 0;

  virtual void OnWriteAscii(StringSpan text) = 0;
  virtual void OnWriteUtf8(StringSpan text) = 0;
  virtual void OnWriteUtf16(String16Span text) = 0;

  virtual void OnWriteEncoded(const BufferSpan& text, TextEncoding encoding) = 0;

  virtual void OnEndLine();

  virtual void OnIndent(int count, char c);

  virtual void OnFlush();

  DISALLOW_COPY_AND_ASSIGN(TextWriter);
};

inline void TextWriter::Write(char c) {
  ASSERT(IsAscii(c));
  OnWriteAsciiChar(c);
}

inline void TextWriter::Write(char32_t c) {
  ASSERT(Unicode::IsValidCodepoint(c));
  OnWriteUnicodeChar(c);
}

inline void TextWriter::WriteAscii(StringSpan text) {
  ASSERT(IsAscii(text));
  OnWriteAscii(text);
}

inline void TextWriter::Indent(int count, char c) {
  ASSERT(IsAscii(c));
  OnIndent(count, c);
}

} // namespace stp

#endif // STP_BASE_IO_TEXTWRITER_H_
