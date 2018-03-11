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

bool ClipTextWriter::grow(int n) {
  if (remaining_ >= n) {
    remaining_ -= n;
  } else {
    n = max(remaining_, 0);
    remaining_ = -1;
  }
  return n;
}

TextEncoding ClipTextWriter::getEncoding() const {
  return base_.getEncoding();
}

void ClipTextWriter::onWriteChar(char c) {
  if (grow(1) > 0)
    base_ << c;
}

void ClipTextWriter::onWriteRune(char32_t rune) {
  if (grow(1) > 0)
    base_ << rune;
}

static bool splitsCharacterAt(StringSpan text, int at) {
  return Utf8::IsEncodedTrail(text[at]);
}

static void trimLastCharacter(StringSpan& text) {
  bool lead = false;
  while (!lead && !text.isEmpty()) {
    lead = Utf8::IsEncodedLead(text.last());
    text.removeSuffix(1);
  }
}

static void cutText(StringSpan& text, int at) {
  bool splits = splitsCharacterAt(text, at);
  text.truncate(at);
  if (splits)
    trimLastCharacter(text);
}

void ClipTextWriter::onWriteString(StringSpan text) {
  int n = grow(text.size());
  if (n < text.size()) {
    if (n > 0)
      cutText(text, n);
    if (n <= 0)
      return;
  }
  base_ << text;
}

void ClipTextWriter::onIndent(int count, char c) {
  int n = grow(count);
  if (n > 0)
    base_.indent(n, c);
}

} // namespace stp
