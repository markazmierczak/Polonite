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

void StringWriter::onWriteChar(char c) {
  string_.add(c);
}

void StringWriter::onWriteRune(char32_t rune) {
  appendRune(string_, rune);
}

void StringWriter::onWriteString(StringSpan text) {
  string_.append(text);
}

void StringWriter::onIndent(int count, char c) {
  ASSERT(count >= 0);
  ASSERT(isAscii(c));
  string_.addRepeat(c, count);
}

} // namespace stp
