// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/InlineStringWriter.h"

#include "Base/Debug/Log.h"
#include "Base/Text/StringUtfConversions.h"
#include "Base/Text/TextDecoder.h"

namespace stp {

TextEncoding InlineStringWriter::GetEncoding() const {
  return BuiltinTextEncoding::Utf8();
}
TextEncoding InlineString16Writer::GetEncoding() const {
  return BuiltinTextEncoding::Utf16();
}

void InlineStringWriter::OnWriteAsciiChar(char c) {
  ASSERT(IsAscii(c));
  string_.Add(c);
}
void InlineString16Writer::OnWriteAsciiChar(char c) {
  ASSERT(IsAscii(c));
  string_.Add(char_cast<char16_t>(c));
}

void InlineStringWriter::OnWriteUnicodeChar(char32_t codepoint) {
  AppendUnicodeCharacter(string_, codepoint);
}
void InlineString16Writer::OnWriteUnicodeChar(char32_t codepoint) {
  AppendUnicodeCharacter(string_, codepoint);
}

void InlineStringWriter::OnWriteAscii(StringSpan text) {
  AppendAscii(string_, text);
}
void InlineString16Writer::OnWriteAscii(StringSpan text) {
  AppendAscii(string_, text);
}

void InlineStringWriter::OnWriteUtf8(StringSpan text) {
  string_.Append(text);
}
void InlineString16Writer::OnWriteUtf16(String16Span text) {
  string_.Append(text);
}

template<typename TSrc, typename TDst>
static inline void ConvertAndAppendString(InlineListBase<TDst>& output, Span<TSrc> input) {
  if (!AppendUnicode(output, input))
    LOG(WARN, "replaced illegal sequence with replacement");
}

void InlineStringWriter::OnWriteUtf16(String16Span text) {
  ConvertAndAppendString(string_, text);
}
void InlineString16Writer::OnWriteUtf8(StringSpan text) {
  ConvertAndAppendString(string_, text);
}

template<typename T>
static inline void OnIndentTmpl(InlineListBase<T>& string, int count, char c) {
  ASSERT(count >= 0);
  ASSERT(IsAscii(c));
  string.AddRepeat(char_cast<T>(c), count);
}

void InlineStringWriter::OnIndent(int count, char c) { OnIndentTmpl(string_, count, c); }
void InlineString16Writer::OnIndent(int count, char c) { OnIndentTmpl(string_, count, c); }

void InlineStringWriter::OnWriteEncoded(const BufferSpan& text, TextEncoding encoding) {
  AppendEncoded(string_, text, encoding);
}
void InlineString16Writer::OnWriteEncoded(const BufferSpan& text, TextEncoding encoding) {
  AppendEncoded(string_, text, encoding);
}

} // namespace stp
