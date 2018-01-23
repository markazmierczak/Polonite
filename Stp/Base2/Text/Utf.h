// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_UTF_H_
#define STP_BASE_TEXT_UTF_H_

#include "Base/Debug/Assert.h"

namespace stp {

namespace detail {

template<typename T>
constexpr T UnicodeFallbackUnit = 0xFFFD;
template<>
constexpr char UnicodeFallbackUnit<char> = '?';


} // namespace detail

class Unicode {
  STATIC_ONLY(Unicode);
 public:
  static constexpr char32_t MaxCodepoint = 0x10FFFFu;
  static constexpr char16_t MinLeadSurrogate = 0xD800u;
  static constexpr char16_t MaxLeadSurrogate = 0xDBFFu;
  static constexpr char16_t MinTrailSurrogate = 0xDC00u;
  static constexpr char16_t MaxTrailSurrogate = 0xDFFFu;
  static constexpr char32_t SurrogateOffset =
      (MinLeadSurrogate << 10u) + MinTrailSurrogate - 0x10000u;

  static constexpr char16_t ReplacementCharacter = 0xFFFD;

  template<typename T>
  static constexpr T FallbackUnit = detail::UnicodeFallbackUnit<T>;

  enum DecodeError : char32_t {
    EndOfStreamError = 0x80000000u,
    InvalidSequenceError = 0x80000001u,
  };

  static bool IsDecodeError(char32_t c) { return (c & 0x80000000u) != 0; }

  // Excludes the surrogate code units [0xD800, 0xDFFF] and
  // code-points larger than 0x10FFFF (the highest code-point allowed).
  // Non-characters and unassigned code-points are allowed.
  static bool IsValidCodepoint(char32_t codepoint);

  // Excludes non-characters [U+FDD0..U+FDEF], and all code-points ending in
  // 0xFFFE or 0xFFFF) from the set of valid characters.
  static bool IsValidCharacter(char32_t codepoint);

  // Accepts code units in [U+D8000..U+DFFF] range.
  static bool IsSurrogate(char32_t code_unit);

  // Accepts code units in [U+D8000..U+DBFF] range.
  static bool IsLeadSurrogate(char32_t code_unit);

  // Accepts code units in [U+DC000..U+DFFF] range.
  static bool IsTrailSurrogate(char32_t code_unit);

  // Checks whether given surrogate is leading.
  // False is returned for trailing surrogate.
  static bool SurrogateIsLeading(char32_t surrogate);

  // Decodes a code-point from surrogate pair.
  // Always succeeds (asserts for valid lead and trail surrogate).
  static char32_t DecodeSurrogatePair(char32_t lead, char32_t trail);
};

class BASE_EXPORT Utf8 : public Unicode {
 public:
  // Maximum number of units needed to encode a single code-point in UTF-8.
  static constexpr int MaxEncodedCodepointLength = 4;

  static bool IsEncodedLead(unsigned char c) { return (c - 0xC0) < 0x3E; }
  static bool IsEncodedTrail(unsigned char c) { return (c & 0xC0) == 0x80; }

  // Returns the size needed to encode given code-point in UTF-8.
  static int EncodedLength(char32_t c);

  // Encodes given code-point as UTF-8 sequence.
  // |out| must be big enough to hold encoded sequence.
  // Use MaxEncodedCodepointLength or EncodedLength() to select buffer size.
  // Returns the number of units written to the output.
  static int Encode(char* out, char32_t c);

  static int EncodeInTwoUnits(char* out, char32_t c);

  // Decodes a single code-point from UTF-8 stream.
  // An error is returned if input stream contains invalid sequence.
  // Use IsDecodeError() to check for an error.
  static char32_t Decode(const char*& it, const char* end);

  static bool Validate(StringSpan input);

  static const uint8_t TrailLengths[256];

 private:
  static char32_t DecodeSlow(const char*& it, const char* end, char32_t c);
  static int EncodeSlow(char* out, char32_t c);
};

class BASE_EXPORT Utf16 : public Unicode {
 public:
  static constexpr int MaxEncodedCodepointLength = 2;

  static int EncodedLength(char32_t c);
  static int Encode(char16_t* s, char32_t c);
  static char32_t Decode(const char16_t*& it, const char16_t* end);

  static bool Validate(String16Span input);

 private:
  static char32_t DecodeSlow(const char16_t*& it, const char16_t* end, char32_t c);
};

inline char32_t DecodeUtf(const char*& it, const char* end) {
  return Utf8::Decode(it, end);
}
inline char32_t DecodeUtf(const char16_t*& it, const char16_t* end) {
  return Utf16::Decode(it, end);
}

inline int EncodeUtf(char* out, char32_t c) {
  return Utf8::Encode(out, c);
}
inline int EncodeUtf(char16_t* out, char32_t c) {
  return Utf16::Encode(out, c);
}

namespace detail {

template<typename T>
struct UtfTmplHelperTmpl;

template<> struct UtfTmplHelperTmpl<char    > { typedef Utf8 Type; };
template<> struct UtfTmplHelperTmpl<char16_t> { typedef Utf16 Type; };

} // namespace detail

template<typename T>
using UtfTmpl = typename detail::UtfTmplHelperTmpl<T>::Type;

inline bool Unicode::IsValidCodepoint(char32_t codepoint) {
  return codepoint < MinLeadSurrogate ||
        (codepoint > MaxTrailSurrogate && codepoint <= MaxCodepoint);
}

inline bool Unicode::IsValidCharacter(char32_t codepoint) {
  return codepoint < MinLeadSurrogate ||
        (codepoint > MaxTrailSurrogate && codepoint < 0xFDD0u) ||
        (codepoint > 0xFDEFu && codepoint <= MaxCodepoint && (codepoint & 0xFFFEu) != 0xFFFEu);
}

inline bool Unicode::IsSurrogate(char32_t code_unit) {
  return (code_unit & 0xFFFFF800) == 0xD800;
}

inline bool Unicode::IsLeadSurrogate(char32_t code_unit) {
  return (code_unit & 0xFFFFFC00) == 0xD800;
}

inline bool Unicode::IsTrailSurrogate(char32_t code_unit) {
  return (code_unit & 0xFFFFFC00) == 0xDC00;
}

inline bool Unicode::SurrogateIsLeading(char32_t surrogate) {
  ASSERT(IsSurrogate(surrogate));
  return (surrogate & 0x400) == 0;
}

inline char32_t Unicode::DecodeSurrogatePair(char32_t lead, char32_t trail) {
  ASSERT(IsLeadSurrogate(lead));
  ASSERT(IsTrailSurrogate(trail));
  return (lead << 10u) + trail - SurrogateOffset;
}

inline int Utf8::EncodedLength(char32_t c) {
  ASSERT(IsValidCodepoint(c));
  return c <= 0x7F ? 1 : (c <= 0x7FF ? 2 : (c <= 0xFFFF ? 3 : 4));
}

inline int Utf8::Encode(char* out, char32_t c) {
  ASSERT(IsValidCodepoint(c));
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

inline char32_t Utf8::Decode(const char*& it, const char* end) {
  ASSERT(it != end);
  char32_t c = static_cast<uint8_t>(*it++);
  return (c <= 0x7F) ? c : DecodeSlow(it, end, c);
}

inline int Utf16::EncodedLength(char32_t c) {
  ASSERT(IsValidCodepoint(c));
  return c <= 0xFFFF ? 1 : 2;
}

inline int Utf16::Encode(char16_t* s, char32_t c) {
  ASSERT(IsValidCodepoint(c));
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

inline char32_t Utf16::Decode(const char16_t*& it, const char16_t* end) {
  ASSERT(it != end);
  char32_t c = *it++;
  return IsSurrogate(c) ? DecodeSlow(it, end, c) : static_cast<char32_t>(c);
}

} // namespace stp

#endif // STP_BASE_TEXT_UTF_H_
