// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/StringWriter.h"

#include "Base/Debug/Log.h"
#include "Base/Text/Codec/Utf8Encoding.h"
#include "Base/Text/Utf.h"

namespace stp {

TextEncoding StringWriter::GetEncoding() const {
  return BuiltinTextEncodings::Utf8();
}

void StringWriter::OnWriteChar(char c) {
  string_.Add(c);
}

void StringWriter::OnWriteRune(char32_t rune) {
  AppendRune(string_, rune);
}

void StringWriter::OnWriteString(StringSpan text) {
  string_.Append(text);
}

void StringWriter::OnIndent(int count, char c) {
  ASSERT(count >= 0);
  ASSERT(isAscii(c));
  string_.AddRepeat(c, count);
}

} // namespace stp
