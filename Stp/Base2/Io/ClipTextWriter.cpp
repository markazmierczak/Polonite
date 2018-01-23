// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/ClipTextWriter.h"

#include "Base/Error/BasicExceptions.h"
#include "Base/Io/InlineStringWriter.h"
#include "Base/Text/Codec/TextCodec.h"

namespace stp {

ClipTextWriter::ClipTextWriter(TextWriter& base, int limit)
    : base_(base), remaining_(limit) {
  ASSERT(limit >= 0);
  if (base_.GetEncoding() == Utf8Codec)
    encoding_ = OutputEncoding::Utf8;
  else if (base_.GetEncoding() == Utf16Codec)
    encoding_ = OutputEncoding::Utf16;
  else
    throw NotSupportedException();
}

bool ClipTextWriter::Grow(int n) {
  if (remaining_ < n) {
    remaining_ = -1;
    return false;
  }
  remaining_ -= n;
  return true;
}

const TextCodec& ClipTextWriter::GetEncoding() const {
  return base_.GetEncoding();
}

void ClipTextWriter::OnWriteAsciiChar(char c) {
  ASSERT(IsAscii(c));
  if (!Grow(1))
    return;
  base_.Write(c);
}

void ClipTextWriter::OnWriteUnicodeChar(char32_t codepoint) {
  int length;
  if (encoding_ == OutputEncoding::Utf8) {
    length = Utf8::EncodedLength(codepoint);
  } else {
    ASSERT(encoding_ == OutputEncoding::Utf8);
    length = Utf16::EncodedLength(codepoint);
  }

  if (!Grow(length))
    return;

  base_.Write(codepoint);
}

void ClipTextWriter::OnWriteAscii(StringSpan text) {
  if (remaining_ < text.size()) {
    if (remaining_ < 0)
      return;
    text.Truncate(remaining_);
    remaining_ = -1;
  }
  base_.WriteAscii(text);
}

// FIXME break at grapheme
// This is inherently wrong to break string at codepoint.
// But at this point we have no grapheme iterator yet :(
static bool SplitsCharacterAt(StringSpan text, int at) {
  return Utf8::IsEncodedTrail(text[at]);
}
static bool SplitsCharacterAt(String16Span text, int at) {
  return Utf16::IsTrailSurrogate(text[at]);
}

static void TrimLastCharacter(StringSpan& text) {
  bool lead = false;
  while (!lead && !text.IsEmpty()) {
    lead = Utf8::IsEncodedLead(text.GetLast());
    text.RemoveSuffix(1);
  }
}

static void TrimLastCharacter(String16Span& text) {
  if (!text.IsEmpty() && Utf16::IsLeadSurrogate(text.GetLast()))
    text.RemoveSuffix(1);
}

template<typename T>
static bool CutText(Span<T> text, int& remaining) {
  if (remaining < 0)
    return false;
  if (remaining >= text.size()) {
    remaining -= text.size();
    return true;
  }
  int at = Exchange(remaining, -1);
  bool splits = SplitsCharacterAt(text, at);
  text.Truncate(at);
  if (splits)
    TrimLastCharacter(text);
  return true;
}

void ClipTextWriter::OnWriteUtf8(StringSpan text) {
  if (encoding_ == OutputEncoding::Utf8) {
    if (CutText(text, remaining_))
      base_.Write(text);
    return;
  }
  ASSERT(encoding_ == OutputEncoding::Utf16);
  const char* it = begin(text);
  const char* it_end = end(text);
  while (it < it_end) {
    const char* prev = it;
    int n;
    char32_t c = DecodeUtf(it, it_end);
    if (Unicode::IsDecodeError(c))
      n = 1;
    else
      n = Utf16::EncodedLength(c);
    if (remaining_ < n) {
      remaining_ = -1;
      it = prev;
      break;
    }
    remaining_ -= n;
  }
  base_.Write(MakeSpan(text.data(), it - text.data()));
}

void ClipTextWriter::OnWriteUtf16(String16Span text) {
  if (encoding_ == OutputEncoding::Utf16) {
    if (CutText(text, remaining_))
      base_.Write(text);
    return;
  }
  ASSERT(encoding_ == OutputEncoding::Utf8);
  const char16_t* it = begin(text);
  const char16_t* it_end = end(text);
  while (it < it_end) {
    const char16_t* prev = it;
    int n;
    char32_t c = DecodeUtf(it, it_end);
    if (Unicode::IsDecodeError(c))
      n = 1;
    else
      n = Utf8::EncodedLength(c);
    if (remaining_ < n) {
      remaining_ = -1;
      it = prev;
      break;
    }
    remaining_ -= n;
  }
  base_.Write(MakeSpan(text.data(), it - text.data()));
}

template<typename T>
static void Rewrite(TextWriter& out, const BufferSpan& text, const TextCodec& encoding) {
  InlineList<T, 256> buffer;
  InlineStringTmplWriter<T> buffer_writer(buffer);
  buffer_writer.Write(text, encoding);
  out.Write(buffer);
}

void ClipTextWriter::OnWriteEncoded(const BufferSpan& text, const TextCodec& encoding) {
  if (remaining_ < 0)
    return;
  if (encoding_ == OutputEncoding::Utf8) {
    Rewrite<char>(*this, text, encoding);
  } else {
    ASSERT(encoding_ == OutputEncoding::Utf16);
    Rewrite<char16_t>(*this, text, encoding);
  }
}

void ClipTextWriter::OnIndent(int count, char c) {
  ASSERT(count >= 0);
  ASSERT(IsAscii(c));
  if (remaining_ < count) {
    if (remaining_ < 0)
      return;
    count = Exchange(remaining_, -1);
  }
  base_.Indent(count, c);
}

} // namespace stp
