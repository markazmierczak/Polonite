// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Utf.h"

#include "Base/Text/StringUtfConversions.h"

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
    return EndOfStreamError;
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
        return InvalidSequenceError;
      }
      return c;

    case 2:
      c &= 0x0F;
      illegal = DecodeOneInUtf8Sequence(it, c);
      illegal |= DecodeOneInUtf8Sequence(it, c);
      if (illegal || c < 0x800 || unicode::IsSurrogate(c)) {
        it = start;
        SkipUtf8Trail(it, end);
        return InvalidSequenceError;
      }
      return c;

    case 1:
      c &= 0x1F;
      illegal = DecodeOneInUtf8Sequence(it, c);
      if (illegal || c < 0x80) {
        it = start;
        SkipUtf8Trail(it, end);
        return InvalidSequenceError;
      }
      return c;

    case 0:
      return InvalidSequenceError;
  }
  UNREACHABLE(return InvalidSequenceError);
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

char32_t Utf16::DecodeSlow(const char16_t*& it, const char16_t* end, char32_t lead) {
  if (it >= end)
    return EndOfStreamError;

  if (!unicode::SurrogateIsLeading(lead))
    return InvalidSequenceError;

  char16_t trail = *it++;

  if (!unicode::IsTrailSurrogate(trail)) {
    // Invalid surrogate pair.
    return InvalidSequenceError;
  }

  // Valid surrogate pair.
  char32_t decoded = unicode::DecodeSurrogatePair(lead, trail);
  ASSERT(unicode::IsValidCodepoint(decoded));
  return decoded;
}

bool Utf8::Validate(StringSpan input) {
  const char* src = input.data();
  const char* end = src + input.size();

  while (src < end) {
    char32_t c = Utf8::Decode(src, end);
    if (!unicode::IsValidCodepoint(c))
      return false;
  }
  return true;
}

bool Utf16::Validate(String16Span input) {
  const char16_t* src = input.data();
  const char16_t* end = src + input.size();

  bool expects_trail = false;
  while (src < end) {
    char16_t c = *src++;
    if (expects_trail) {
      if (!unicode::IsTrailSurrogate(c))
        return false;
    } else {
      if (unicode::IsSurrogate(c)) {
        if (!unicode::SurrogateIsLeading(c))
          return false;
        expects_trail = true;
      }
    }
  }
  return !expects_trail;
}

} // namespace stp
