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
  bool hasReachedLimit() const { return remaining_ < 0; }

  TextEncoding getEncoding() const override;

 protected:
  void onWriteChar(char c) override;
  void onWriteRune(char32_t rune) override;
  void onWriteString(StringSpan text) override;
  void onIndent(int count, char c) override;

 private:
  bool grow(int n);

  TextWriter& base_;
  int remaining_;
};

} // namespace stp

#endif // STP_BASE_IO_CLIPTEXTWRITER_H_
