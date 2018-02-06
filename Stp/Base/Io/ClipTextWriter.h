// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_CLIPTEXTWRITER_H_
#define STP_BASE_IO_CLIPTEXTWRITER_H_

#include "Base/Io/TextWriter.h"

namespace stp {

// ClipTextWriter maintains the limit of characters - cuts off the text
// if limit is reached.
// It is designed for ASCII text only.
// It counts only units, not codepoints or graphemes from Unicode.
// It maintains codepoint integrity though.
// ClipTextWriter knows nothing about graphemes and may cut in the middle of them.
class BASE_EXPORT ClipTextWriter final : public TextWriter {
 public:
  ClipTextWriter(TextWriter* base, int limit);

  // Returns true if any write was cut off due the limit.
  bool ReachedLimit() const { return remaining_ < 0; }

  TextEncoding GetEncoding() const override;

 protected:
  void OnWriteAsciiChar(char c) override;
  void OnWriteUnicodeChar(char32_t c) override;
  void OnWriteAscii(StringSpan text) override;
  void OnWriteUtf8(StringSpan text) override;
  void OnWriteUtf16(String16Span text) override;
  void OnIndent(int count, char c) override;

 private:
  bool Grow(int n);

  TextWriter& base_;
  int remaining_;
};

} // namespace stp

#endif // STP_BASE_IO_CLIPTEXTWRITER_H_
