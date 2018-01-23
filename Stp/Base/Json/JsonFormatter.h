// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_JSON_JSONFORMATTER_H_
#define STP_BASE_JSON_JSONFORMATTER_H_

#include "Base/Json/JsonArray.h"
#include "Base/Json/JsonError.h"
#include "Base/Json/JsonObject.h"

namespace stp {

class BASE_EXPORT JsonFormatter {
 public:
  explicit JsonFormatter(TextWriter& out) : out_(out) {}

  bool Write(const JsonValue& root);

  void SetOptions(const JsonOptions& options) { options_ = options; }

  // Appends to |dest| an escaped version of |str|. Valid UTF-8 code units will
  // pass through from the input to the output. Invalid code units will be
  // replaced with the U+FFFD replacement character. This function returns true
  // if no replacement was necessary and false if there was a lossy replacement.
  //
  // If |escape_unicode| is true, then all non-ASCII characters are encoded
  // using \uXXXX escape sequence.
  static bool Escape(TextWriter& out, StringSpan str, bool escape_unicode = false);

 private:
  // Called recursively to build the JSON string.
  // Unlike Write(root) this one returns false when parser stopped
  // (not when an error occurred).
  bool Write(const JsonValue& node, int depth);

  void IndentLine(int depth);

  bool WriteInteger(int64_t x);
  bool WriteDouble(const JsonValue& node);
  bool WriteString(StringSpan str);
  bool WriteArray(const JsonArray& array, int depth);
  bool WriteObject(const JsonObject& object, int depth);

  // Returns true if formatting should be stopped.
  bool RaiseError(JsonError::Code code);

  bool PrintsPretty() const { return options_.Has(JsonOptions::PrettyFormatting); }

  // Where we write JSON data as we generate it.
  TextWriter& out_;

  JsonOptions options_;

  JsonError error_;

  static void EscapeSimple(TextWriter& out, StringSpan str);
  static bool EscapeReplaceUnicode(TextWriter& out, StringSpan str);

  DISALLOW_COPY_AND_ASSIGN(JsonFormatter);
};

} // namespace stp

#endif // STP_BASE_JSON_JSONFORMATTER_H_
