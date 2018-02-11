// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/StringWriter.h"

#include "Base/Debug/Log.h"
#include "Base/Text/Codec/Utf8Encoding.h"
#include "Base/Text/StringUtfConversions.h"

namespace stp {

TextEncoding StringWriter::GetEncoding() const {
  return BuiltinTextEncodings::Utf8();
}

void StringWriter::OnWriteAsciiChar(char c) {
  string_.Add(c);
}

void StringWriter::OnWriteAscii(StringSpan text) {
  AppendAscii(string_, text);
}

void StringWriter::OnIndent(int count, char c) {
  ASSERT(count >= 0);
  ASSERT(IsAscii(c));
  string_.AddRepeat(c, count);
}

void StringWriter::OnWriteUnicodeChar(char32_t codepoint) {
  AppendUnicodeCharacter(string_, codepoint);
}

void StringWriter::OnWriteUtf8(StringSpan text) {
  string_.Append(text);
}

void StringWriter::OnWriteUtf16(String16Span text) {
  if (!AppendUnicode(string_, text))
    LOG(WARN, "replaced illegal sequence with replacement");
}

} // namespace stp
