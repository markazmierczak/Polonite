// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/InlineStringWriter.h"

#include "Base/Debug/Log.h"
#include "Base/Text/Codec/Utf8Encoding.h"
#include "Base/Text/StringUtfConversions.h"

namespace stp {

TextEncoding InlineStringWriter::GetEncoding() const {
  return BuiltinTextEncodings::Utf8();
}

void InlineStringWriter::OnWriteAsciiChar(char c) {
  ASSERT(IsAscii(c));
  string_.Add(c);
}

void InlineStringWriter::OnWriteUnicodeChar(char32_t codepoint) {
  AppendUnicodeCharacter(string_, codepoint);
}

void InlineStringWriter::OnWriteAscii(StringSpan text) {
  AppendAscii(string_, text);
}

void InlineStringWriter::OnWriteUtf8(StringSpan text) {
  string_.Append(text);
}

void InlineStringWriter::OnWriteUtf16(String16Span text) {
  if (!AppendUnicode(string_, text))
    LOG(WARN, "replaced illegal sequence with replacement");
}

void InlineStringWriter::OnIndent(int count, char c) {
  ASSERT(count >= 0);
  ASSERT(IsAscii(c));
  string_.AddRepeat(c, count);
}

} // namespace stp
