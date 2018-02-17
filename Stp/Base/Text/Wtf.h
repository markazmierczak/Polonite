// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_WTF_H_
#define STP_BASE_TEXT_WTF_H_

#include "Base/Containers/ListFwd.h"
#include "Base/Text/Utf.h"

namespace stp {

#define HAVE_UTF8_NATIVE_VALIDATION (OS(ANDROID) || OS(DARWIN))

// There are systems (POSIX, Windows) where filenames are not validated for specific encoding.
// It may have any value in given representation (byte or wide).
//
// Therefore we introduce WTF-8/16 encodings to deal with potentially ill-formed
// UTF-8/16. Unpaired surrogate are valid code-points in this encoding.
//
// https://simonsapin.github.io/wtf-8

class BASE_EXPORT Wtf8 {
 public:
  // Maximum number of units needed to encode a single code-point in UTF-8.
  static constexpr int MaxEncodedRuneLength = 4;

  static bool IsEncodedLead(unsigned char c) { return (c - 0xC0) < 0x3E; }
  static bool IsEncodedTrail(unsigned char c) { return (c & 0xC0) == 0x80; }

  // Returns the size needed to encode given code-point in UTF-8.
  static int EncodedLength(char32_t c);

  // Encodes given code-point as WTF-8 sequence.
  // |out| must be big enough to hold encoded sequence.
  // Use MaxEncodedRuneLength or EncodedLength() to select buffer size.
  // Returns the number of units written to the output.
  static int Encode(char* out, char32_t c);

  // Decodes a single code-point from WTF-8 stream.
  // An error is returned if input stream contains invalid sequence.
  // Use IsDecodeError() to check for an error.
  static char32_t Decode(const char*& it, const char* end);

 private:
  static char32_t DecodeSlow(const char*& it, const char* end, char32_t c);
};

class BASE_EXPORT Wtf16 {
 public:
  static constexpr int MaxEncodedRuneLength = 2;

  static int EncodedLength(char32_t c);
  static int Encode(char16_t* s, char32_t c);
  static char32_t Decode(const char16_t*& it, const char16_t* end);

 private:
  static char32_t DecodeSlow(const char16_t*& it, const char16_t* end, char32_t c);
};

// Converts WTF to strict UTF and writes to the output.
// This conversion is lossy.
BASE_EXPORT void WriteWtf(TextWriter& out, StringSpan wtf8);

BASE_EXPORT String WtfToUtf8(StringSpan wtf8);

BASE_EXPORT void AppendWtf(String& output, StringSpan wtf);

inline int Wtf8::EncodedLength(char32_t c) {
  ASSERT(c <= unicode::MaxRune);
  return c <= 0x7F ? 1 : (c <= 0x7FF ? 2 : (c <= 0xFFFF ? 3 : 4));
}

inline int Wtf8::Encode(char* out, char32_t c) {
  ASSERT(c <= unicode::MaxRune);
  int i = 0;
  if (c <= 0x7F) {
    out[i++] = static_cast<uint8_t>(c);
  } else {
    if (c <= 0x7FF) {
      out[i++] = static_cast<uint8_t>((c >> 6) | 0xC0);
    } else {
      if (c <= 0xFFFF) {
        out[i++] = static_cast<uint8_t>((c >> 12) | 0xE0);
      } else {
        out[i++] = static_cast<uint8_t>((c >> 18) | 0xF0);
        out[i++] = static_cast<uint8_t>(((c >> 12) & 0x3F) | 0x80);
      }
      out[i++] = static_cast<uint8_t>(((c >> 6) & 0x3F) | 0x80);
    }
    out[i++] = static_cast<uint8_t>((c & 0x3F) | 0x80);
  }
  return i;
}

inline char32_t Wtf8::Decode(const char*& it, const char* end) {
  ASSERT(it != end);
  char32_t c = static_cast<uint8_t>(*it++);
  return (c <= 0x7F) ? static_cast<char32_t>(c) : DecodeSlow(it, end, c);
}

inline int Wtf16::EncodedLength(char32_t c) {
  ASSERT(c <= unicode::MaxRune);
  return c <= 0xFFFF ? 1 : 2;
}

inline int Wtf16::Encode(char16_t* s, char32_t c) {
  ASSERT(c <= unicode::MaxRune);
  int i = 0;
  if (c <= 0xFFFF) {
    // This code point is in the Basic Multilingual Plane (BMP) or surrogate.
    s[i++] = static_cast<uint16_t>(c);
  } else {
    // Non-BMP characters use a double-character encoding.
    s[i++] = static_cast<uint16_t>((c >> 10) + 0xD7C0);
    s[i++] = static_cast<uint16_t>((c & 0x03FF) | 0xDC00);
  }
  return i;
}

inline char32_t Wtf16::Decode(const char16_t*& it, const char16_t* end) {
  ASSERT(it != end);
  char32_t c = *it++;
  return unicode::IsSurrogate(c) ? DecodeSlow(it, end, c) : static_cast<char32_t>(c);
}

} // namespace stp

#endif // STP_BASE_TEXT_WTF_H_
