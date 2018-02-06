// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/ClipTextWriter.h"

#include "Base/Error/BasicExceptions.h"
#include "Base/Io/InlineStringWriter.h"
#include "Base/Text/TextEncoding.h"
#include "Base/Text/Utf.h"

namespace stp {

ClipTextWriter::ClipTextWriter(TextWriter* base, int limit)
    : base_(*base), remaining_(limit) {
  ASSERT(limit >= 0);
}

bool ClipTextWriter::Grow(int n) {
  if (remaining_ >= n) {
    remaining_ -= n;
  } else {
    n = Max(remaining_, 0);
    remaining_ = -1;
  }
  return n;
}

TextEncoding ClipTextWriter::GetEncoding() const {
  return base_.GetEncoding();
}

void ClipTextWriter::OnWriteAsciiChar(char c) {
  ASSERT(IsAscii(c));
  if (Grow(1) > 0)
    base_.Write(c);
}

void ClipTextWriter::OnWriteUnicodeChar(char32_t codepoint) {
  if (Grow(1) > 0)
    base_.Write(codepoint);
}

void ClipTextWriter::OnWriteAscii(StringSpan text) {
  int n = Grow(text.size());
  if (n > 0)
    base_.WriteAscii(text.GetSlice(0, n));
}

static bool SplitsCharacterAt(StringSpan text, int at) {
  return Utf8::IsEncodedTrail(text[at]);
}
static bool SplitsCharacterAt(String16Span text, int at) {
  return unicode::IsTrailSurrogate(text[at]);
}

static void TrimLastCharacter(StringSpan& text) {
  bool lead = false;
  while (!lead && !text.IsEmpty()) {
    lead = Utf8::IsEncodedLead(text.GetLast());
    text.RemoveSuffix(1);
  }
}

static void TrimLastCharacter(String16Span& text) {
  if (!text.IsEmpty() && unicode::IsLeadSurrogate(text.GetLast()))
    text.RemoveSuffix(1);
}

template<typename T>
static void CutText(Span<T>& text, int at) {
  bool splits = SplitsCharacterAt(text, at);
  text.Truncate(at);
  if (splits)
    TrimLastCharacter(text);
}

template<typename T>
static void OnWriteUtfTmpl(TextWriter& base, Span<T> text, int n) {
  if (n < text.size()) {
    if (n > 0)
      CutText(text, n);
    if (n <= 0)
      return;
  }
  base.Write(text);
}

void ClipTextWriter::OnWriteUtf8(StringSpan text) {
  int n = Grow(text.size());
  OnWriteUtfTmpl(base_, text, n);
}

void ClipTextWriter::OnWriteUtf16(String16Span text) {
  int n = Grow(text.size());
  OnWriteUtfTmpl(base_, text, n);
}

void ClipTextWriter::OnIndent(int count, char c) {
  int n = Grow(count);
  if (n > 0)
    base_.Indent(n, c);
}

} // namespace stp
