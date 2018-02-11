// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_TEXTWRITER_H_
#define STP_BASE_IO_TEXTWRITER_H_

#include "Base/Containers/BufferSpanFwd.h"
#include "Base/Text/FormatFwd.h"
#include "Base/Text/StringSpan.h"

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
  void Write(char32_t rune);

  void Write(StringSpan text) { OnWriteString(text); }

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
  virtual void OnWriteChar(char c);
  virtual void OnWriteRune(char32_t c);
  virtual void OnWriteString(StringSpan text) = 0;

  virtual void OnEndLine();

  virtual void OnIndent(int count, char c);

  virtual void OnFlush();

  #if ASSERT_IS_ON()
  bool IsValidChar(char c);
  #endif

  DISALLOW_COPY_AND_ASSIGN(TextWriter);
};

inline void TextWriter::Write(char c) {
  ASSERT(IsValidChar(c));
  OnWriteChar(c);
}

inline void TextWriter::Indent(int count, char c) {
  ASSERT(IsValidChar(c));
  OnIndent(count, c);
}

} // namespace stp

#endif // STP_BASE_IO_TEXTWRITER_H_
