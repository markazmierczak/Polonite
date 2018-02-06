// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/StringWriter.h"

#include "Base/Debug/Log.h"
#include "Base/Text/StringUtfConversions.h"
#include "Base/Text/TextDecoder.h"

namespace stp {

TextEncoding StringWriter::GetEncoding() const { return TextEncoding::BuiltinUtf8(); }
TextEncoding String16Writer::GetEncoding() const { return TextEncoding::BuiltinUtf16LE(); }

void StringWriter::OnWriteAsciiChar(char c) { string_.Add(c); }
void String16Writer::OnWriteAsciiChar(char c) { string_.Add(char_cast<char16_t>(c)); }

void StringWriter::OnWriteAscii(StringSpan text) { AppendAscii(string_, text); }
void String16Writer::OnWriteAscii(StringSpan text) { AppendAscii(string_, text); }

template<typename T>
static inline void OnIndentTmpl(List<T>& string, int count, char c) {
  ASSERT(count >= 0);
  ASSERT(IsAscii(c));
  string.AddRepeat(char_cast<T>(c), count);
}

void StringWriter::OnIndent(int count, char c) { OnIndentTmpl(string_, count, c); }
void String16Writer::OnIndent(int count, char c) { OnIndentTmpl(string_, count, c); }

void StringWriter::OnWriteUnicodeChar(char32_t codepoint) {
  AppendUnicodeCharacter(string_, codepoint);
}

void String16Writer::OnWriteUnicodeChar(char32_t codepoint) {
  AppendUnicodeCharacter(string_, codepoint);
}

template<typename TDst, typename TSrc>
static inline void ConvertAndAppendString(TDst& output, const TSrc& input) {
  if (!AppendUnicode(output, input))
    LOG(WARN, "replaced illegal sequence with replacement");
}

void StringWriter::OnWriteUtf8(StringSpan text) {
  string_.Append(text);
}

void String16Writer::OnWriteUtf8(StringSpan text) {
  ConvertAndAppendString(string_, text);
}

void StringWriter::OnWriteUtf16(String16Span text) {
  ConvertAndAppendString(string_, text);
}

void String16Writer::OnWriteUtf16(String16Span text) {
  string_.Append(text);
}

} // namespace stp
