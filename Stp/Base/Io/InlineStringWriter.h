// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_INLINESTRINGWRITER_H_
#define STP_BASE_IO_INLINESTRINGWRITER_H_

#include "Base/Containers/InlineList.h"
#include "Base/Io/TextWriter.h"

namespace stp {

class BASE_EXPORT InlineStringWriter final : public TextWriter {
 public:
  explicit InlineStringWriter(InlineListBase<char>* string) : string_(*string) {}

  TextEncoding getEncoding() const override;

 protected:
  void onWriteChar(char c) override;
  void onWriteRune(char32_t rune) override;
  void onWriteString(StringSpan text) override;
  void onIndent(int count, char c) override;

 private:
  InlineListBase<char>& string_;
};

} // namespace stp

#endif // STP_BASE_IO_INLINESTRINGWRITER_H_
