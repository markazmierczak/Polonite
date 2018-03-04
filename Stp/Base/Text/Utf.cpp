// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Utf.h"

namespace stp {

const uint8_t Utf8::TrailLengths[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static inline unsigned DecodeOneInUtf8Sequence(const char*& it, char32_t& c) {
  uint8_t trail = *it++;
  c = (c << 6) | (trail & 0x3F);
  return (trail & 0xC0) ^ 0x80;
}

static inline void SkipUtf8Trail(const char*& it, const char* end) {
  while (it < end && Utf8::IsEncodedTrail(*it))
    ++it;
}

char32_t Utf8::DecodeSlow(const char*& it, const char* end, char32_t c) {
  ASSERT(c >= 0x80);

  unsigned len = TrailLengths[c];
  ASSERT(len <= 3);

  if (UNLIKELY(it + len > end)) {
    SkipUtf8Trail(it, end);
    return unicode::EndOfStreamRune;
  }

  const char* start = it;

  unsigned illegal;
  switch (len) {
    case 3:
      c &= 0x07;

      illegal = DecodeOneInUtf8Sequence(it, c);
      if (c >= 0x110)
        illegal = 1;
      illegal |= DecodeOneInUtf8Sequence(it, c);
      illegal |= DecodeOneInUtf8Sequence(it, c);
      if (illegal || c < 0x10000) {
        it = start;
        SkipUtf8Trail(it, end);
        return unicode::InvalidSequenceRune;
      }
      return c;

    case 2:
      c &= 0x0F;
      illegal = DecodeOneInUtf8Sequence(it, c);
      illegal |= DecodeOneInUtf8Sequence(it, c);
      if (illegal || c < 0x800 || unicode::IsSurrogate(c)) {
        it = start;
        SkipUtf8Trail(it, end);
        return unicode::InvalidSequenceRune;
      }
      return c;

    case 1:
      c &= 0x1F;
      illegal = DecodeOneInUtf8Sequence(it, c);
      if (illegal || c < 0x80) {
        it = start;
        SkipUtf8Trail(it, end);
        return unicode::InvalidSequenceRune;
      }
      return c;

    case 0:
      return unicode::InvalidSequenceRune;
  }
  UNREACHABLE(return unicode::InvalidSequenceRune);
}

int Utf8::EncodeSlow(char* out, char32_t c) {
  int i = 0;
  if (c <= 0x7FF) {
    out[i++] = static_cast<char>((c >> 6) | 0xC0);
  } else {
    if (c <= 0xFFFF) {
      out[i++] = static_cast<char>((c >> 12) | 0xE0);
    } else {
      out[i++] = static_cast<char>((c >> 18) | 0xF0);
      out[i++] = static_cast<char>(((c >> 12) & 0x3F) | 0x80);
    }
    out[i++] = static_cast<char>(((c >> 6) & 0x3F) | 0x80);
  }
  out[i++] = static_cast<char>((c & 0x3F) | 0x80);
  return i;
}

int TryEncodeUtf(char32_t c, MutableSpan<char> out) {
  ASSERT(unicode::IsValidRune(c));
  if (out.isEmpty())
    return 0;
  int i = 0;
  if (isAscii(c)) {
    out[i++] = static_cast<char>(c);
  } else {
    if (out.size() < 2)
      return 0;
    if (c <= 0x7FF) {
      out[i++] = static_cast<char>((c >> 6) | 0xC0);
    } else {
      if (out.size() < 3)
        return 0;
      if (c <= 0xFFFF) {
        out[i++] = static_cast<char>((c >> 12) | 0xE0);
      } else {
        if (out.size() < 4)
          return 0;
        out[i++] = static_cast<char>((c >> 18) | 0xF0);
        out[i++] = static_cast<char>(((c >> 12) & 0x3F) | 0x80);
      }
      out[i++] = static_cast<char>(((c >> 6) & 0x3F) | 0x80);
    }
    out[i++] = static_cast<char>((c & 0x3F) | 0x80);
  }
  return i;
}

char32_t Utf16::DecodeSlow(const char16_t*& it, const char16_t* end, char32_t lead) {
  if (it >= end)
    return unicode::EndOfStreamRune;

  if (!unicode::SurrogateIsLeading(lead))
    return unicode::InvalidSequenceRune;

  char16_t trail = *it++;

  if (!unicode::IsTrailSurrogate(trail)) {
    // Invalid surrogate pair.
    return unicode::InvalidSequenceRune;
  }

  // Valid surrogate pair.
  char32_t decoded = unicode::DecodeSurrogatePair(lead, trail);
  ASSERT(unicode::IsValidRune(decoded));
  return decoded;
}

} // namespace stp
