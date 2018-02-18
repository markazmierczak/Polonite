// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Json/JsonError.h"

#include "Base/Io/TextWriter.h"

namespace stp {

void JsonError::ToFormat(TextWriter& out, const StringSpan& opts) const {
  if (line > 0) {
    out << '[';
    out << line;
    if (column > 0) {
      out << ':';
      out << column;
    }
    out << "] ";
  }
  out << CodeToMessage(code);
}

static constexpr StringSpan Messages[] = {
  "ok",
  "invalid escape sequence",
  "syntax error",
  "unexpected token",
  "trailing comma not allowed",
  "too much nesting",
  "unexpected data after root element",
  "unsupported encoding",
  "object keys must be quoted",
  "loss of precision",
  "invalid number",
  "key already assigned",
};
static_assert(isizeofArray(Messages) == JsonError::CodeCount, "!");

StringSpan JsonError::CodeToMessage(Code code) {
  return Messages[code];
}

} // namespace stp
