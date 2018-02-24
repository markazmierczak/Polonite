// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_STRINGWRITER_H_
#define STP_BASE_IO_STRINGWRITER_H_

#include "Base/Io/TextWriter.h"
#include "Base/String/String.h"

namespace stp {

class BASE_EXPORT StringWriter final : public TextWriter {
 public:
  explicit StringWriter(String* string) : string_(*string) {}

  TextEncoding getEncoding() const override;

 protected:
  void onWriteChar(char c) override;
  void onWriteRune(char32_t rune) override;
  void onWriteString(StringSpan text) override;
  void onIndent(int count, char c) override;

 private:
  String& string_;
};

} // namespace stp

#endif // STP_BASE_IO_STRINGWRITER_H_
