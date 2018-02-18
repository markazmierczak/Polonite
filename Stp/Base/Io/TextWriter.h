// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_TEXTWRITER_H_
#define STP_BASE_IO_TEXTWRITER_H_

#include "Base/Containers/BufferSpanFwd.h"
#include "Base/Containers/Span.h"

namespace stp {

class TextEncoding;

// Locale-independent replacement for std::ostream.
// Useful for writing text-based representation of data, for example XML and JSON.
// it should NOT be used to build text displayed to the user since it does not
// support locale (and will not do; on purpose).
class BASE_EXPORT TextWriter {
 public:
  static TextWriter& Null();

  TextWriter() {}
  virtual ~TextWriter() {}

  void Indent(int count, char c = ' ');

  void Flush() { OnFlush(); }

  virtual TextEncoding GetEncoding() const = 0;

  // Simple RTTI:
  virtual bool IsConsoleWriter() const;

  friend TextWriter& operator<<(TextWriter& out, char c) {
    ASSERT(TextWriter::IsValidChar(c));
    out.OnWriteChar(c);
    return out;
  }
  friend TextWriter& operator<<(TextWriter& out, char32_t rune) {
    out.WriteRune(rune);
    return out;
  }
  friend TextWriter& operator<<(TextWriter& out, StringSpan text) {
    out.OnWriteString(text);
    return out;
  }

 protected:
  virtual void OnWriteChar(char c);
  virtual void OnWriteRune(char32_t rune);
  virtual void OnWriteString(StringSpan text) = 0;

  virtual void OnEndLine();

  virtual void OnIndent(int count, char c);

  virtual void OnFlush();

 private:
  #if ASSERT_IS_ON
  static bool IsValidChar(char c);
  #endif

  void WriteRune(char32_t rune);

  DISALLOW_COPY_AND_ASSIGN(TextWriter);
};

inline void TextWriter::Indent(int count, char c) {
  ASSERT(IsValidChar(c));
  OnIndent(count, c);
}

} // namespace stp

#endif // STP_BASE_IO_TEXTWRITER_H_
