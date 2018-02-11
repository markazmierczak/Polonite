// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_STRINGWRITER_H_
#define STP_BASE_IO_STRINGWRITER_H_

#include "Base/Containers/List.h"
#include "Base/Io/TextWriter.h"

namespace stp {

class BASE_EXPORT StringWriter final : public TextWriter {
 public:
  explicit StringWriter(String* string) : string_(*string) {}

  TextEncoding GetEncoding() const override;

 protected:
  void OnWriteChar(char c) override;
  void OnWriteRune(char32_t rune) override;
  void OnWriteString(StringSpan text) override;
  void OnIndent(int count, char c) override;

 private:
  String& string_;
};

} // namespace stp

#endif // STP_BASE_IO_STRINGWRITER_H_
