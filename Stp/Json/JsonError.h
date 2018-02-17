// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_JSON_JSONERROR_H_
#define STP_BASE_JSON_JSONERROR_H_

#include "Base/Containers/Span.h"

namespace stp {

struct BASE_EXPORT JsonError {
  enum Code {
    Ok,
    InvalidEscape,
    SyntaxError,
    UnexpectedToken,
    TrailingComma,
    TooMuchNesting,
    UnexpectedDataAfterRoot,
    UnsupportedEncoding,
    UnquotedObjectKey,
    LossOfPrecision,
    InvalidNumber,
    KeyAlreadyAssigned,
  };
  static constexpr int CodeCount = KeyAlreadyAssigned + 1;

  JsonError() : code(Ok) {}
  explicit JsonError(Code code) : code(code) {}

  JsonError(Code code, int line, int column)
      : code(code), line(line), column(column) {}

  Code code;

  int line = 0;
  int column = 0;

  bool operator==(const JsonError& other) const;
  bool operator!=(const JsonError& other) const { return !operator==(other); }

  static StringSpan CodeToMessage(Code code);
};

inline bool JsonError::operator==(const JsonError& other) const {
  return code == other.code && line == other.line && column == other.column;
}

} // namespace stp

#endif // STP_BASE_JSON_JSONERROR_H_
