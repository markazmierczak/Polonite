// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_UTF_H_
#define STP_BASE_TEXT_UTF_H_

#include "Base/Containers/Span.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/Unicode.h"

namespace stp {

class BASE_EXPORT Utf8 {
 public:
  // Maximum number of units needed to encode a single code-point in UTF-8.
  static constexpr int MaxEncodedRuneLength = 4;

  static bool IsEncodedLead(unsigned char c) { return static_cast<uint8_t>(c - 0xC0) < 0x3E; }
  static bool IsEncodedTrail(unsigned char c) { return (c & 0xC0) == 0x80; }

  // Returns the size needed to encode given code-point in UTF-8.
  static int EncodedLength(char32_t c);

  // Encodes given code-point as UTF-8 sequence.
  // |out| must be big enough to hold encoded sequence.
  // Use MaxEncodedRuneLength or EncodedLength() to select buffer size.
  // Returns the number of units written to the output.
  static int Encode(char* out, char32_t c);

  static int EncodeInTwoUnits(char* out, char32_t c);

  // Decodes a single code-point from UTF-8 stream.
  // An error is returned if input stream contains invalid sequence.
  // Use IsDecodeError() to check for an error.
  static char32_t TryDecode(const char*& it, const char* end);

  static const uint8_t TrailLengths[256];

 private:
  static char32_t DecodeSlow(const char*& it, const char* end, char32_t c);
  static int EncodeSlow(char* out, char32_t c);
};

class BASE_EXPORT Utf16 {
 public:
  static constexpr int MaxEncodedRuneLength = 2;

  static int EncodedLength(char32_t c);
  static int Encode(char16_t* s, char32_t c);
  static char32_t TryDecode(const char16_t*& it, const char16_t* end);

 private:
  static char32_t DecodeSlow(const char16_t*& it, const char16_t* end, char32_t c);
};

inline char32_t TryDecodeUtf(const char*& it, const char* end) {
  return Utf8::TryDecode(it, end);
}
inline char32_t TryDecodeUtf(const char16_t*& it, const char16_t* end) {
  return Utf16::TryDecode(it, end);
}

inline int EncodeUtf(char* out, char32_t c) {
  return Utf8::Encode(out, c);
}
inline int EncodeUtf(char16_t* out, char32_t c) {
  return Utf16::Encode(out, c);
}

BASE_EXPORT int TryEncodeUtf(char32_t c, MutableStringSpan out);

inline int Utf8::EncodedLength(char32_t c) {
  ASSERT(unicode::IsValidRune(c));
  return c <= 0x7F ? 1 : (c <= 0x7FF ? 2 : (c <= 0xFFFF ? 3 : 4));
}

inline int Utf8::Encode(char* out, char32_t c) {
  ASSERT(unicode::IsValidRune(c));
  if (c <= 0x7F) {
    *out = static_cast<char>(c);
    return 1;
  }
  return EncodeSlow(out, c);
}

inline int Utf8::EncodeInTwoUnits(char* out, char32_t c) {
  ASSERT(0x80 <= c && c < 0x800);
  out[0] = static_cast<char>((c >> 6) | 0xC0);
  out[1] = static_cast<char>((c & 0x3F) | 0x80);
  return 2;
}

inline char32_t Utf8::TryDecode(const char*& it, const char* end) {
  ASSERT(it != end);
  char32_t c = static_cast<uint8_t>(*it++);
  return (c <= 0x7F) ? c : DecodeSlow(it, end, c);
}

inline int Utf16::EncodedLength(char32_t c) {
  ASSERT(unicode::IsValidRune(c));
  return c <= 0xFFFF ? 1 : 2;
}

inline int Utf16::Encode(char16_t* s, char32_t c) {
  ASSERT(unicode::IsValidRune(c));
  int i = 0;
  if (c <= 0xFFFF) {
    // This code point is in the Basic Multilingual Plane (BMP).
    s[i++] = static_cast<uint16_t>(c);
  } else {
    // Non-BMP characters use a double-character encoding.
    s[i++] = static_cast<uint16_t>((c >> 10) + 0xD7C0);
    s[i++] = static_cast<uint16_t>((c & 0x03FF) | 0xDC00);
  }
  return i;
}

inline char32_t Utf16::TryDecode(const char16_t*& it, const char16_t* end) {
  ASSERT(it != end);
  char32_t c = *it++;
  return unicode::IsSurrogate(c) ? DecodeSlow(it, end, c) : static_cast<char32_t>(c);
}

template<typename TOutput>
inline int AppendRune(TOutput& output, char32_t rune) {
  using CharType = typename TOutput::ItemType;
  if (sizeof(CharType) == 1 && isAscii(rune)) {
    // Fast path the common case of one byte.
    output.Add(static_cast<char>(rune));
    return 1;
  }
  auto* dst = output.AppendUninitialized(4);
  int n = EncodeUtf(dst, rune);
  output.removeSuffix(4 - n);
  return n;
}

} // namespace stp

#endif // STP_BASE_TEXT_UTF_H_
