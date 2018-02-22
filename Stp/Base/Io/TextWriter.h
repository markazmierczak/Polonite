// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_TEXTWRITER_H_
#define STP_BASE_IO_TEXTWRITER_H_

#include "Base/Containers/Span.h"

namespace stp {

class TextEncoding;

class BASE_EXPORT TextWriter {
 public:
  static TextWriter& nullWriter();

  TextWriter() {}
  virtual ~TextWriter() {}

  void indent(int count, char c = ' ');

  void flush() { onFlush(); }

  virtual TextEncoding getEncoding() const = 0;

  // Simple RTTI:
  virtual bool isConsoleWriter() const;

  friend TextWriter& operator<<(TextWriter& out, char c) {
    ASSERT(TextWriter::isValidChar(c));
    out.onWriteChar(c);
    return out;
  }
  friend TextWriter& operator<<(TextWriter& out, char32_t rune) {
    out.writeRune(rune);
    return out;
  }
  friend TextWriter& operator<<(TextWriter& out, StringSpan text) {
    out.onWriteString(text);
    return out;
  }

 protected:
  virtual void onWriteChar(char c);
  virtual void onWriteRune(char32_t rune);
  virtual void onWriteString(StringSpan text) = 0;

  virtual void onEndLine();

  virtual void onIndent(int count, char c);

  virtual void onFlush();

 private:
  #if ASSERT_IS_ON
  static bool isValidChar(char c);
  #endif

  void writeRune(char32_t rune);

  DISALLOW_COPY_AND_ASSIGN(TextWriter);
};

inline void TextWriter::indent(int count, char c) {
  ASSERT(isValidChar(c));
  onIndent(count, c);
}

} // namespace stp

#endif // STP_BASE_IO_TEXTWRITER_H_
