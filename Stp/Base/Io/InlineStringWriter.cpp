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

void InlineStringWriter::OnWriteChar(char c) {
  string_.Add(c);
}

void InlineStringWriter::OnWriteRune(char32_t rune) {
  AppendUnicodeCharacter(string_, rune);
}

void InlineStringWriter::OnWriteString(StringSpan text) {
  string_.Append(text);
}

void InlineStringWriter::OnIndent(int count, char c) {
  ASSERT(count >= 0);
  ASSERT(IsAscii(c));
  string_.AddRepeat(c, count);
}

} // namespace stp
