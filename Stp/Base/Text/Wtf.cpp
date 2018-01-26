// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Wtf.h"

#include "Base/Io/StringWriter.h"

namespace stp {

static inline unsigned DecodeOneInWtf8Sequence(const char*& it, char32_t& c) {
  uint8_t trail = *it++;
  c = (c << 6) | (trail & 0x3F);
  return (trail & 0xC0) ^ 0x80;
}

static inline void SkipWtf8Trail(const char*& it, const char* end) {
  while (it < end && Wtf8::IsEncodedTrail(*it))
    ++it;
}

char32_t Wtf8::DecodeSlow(const char*& it, const char* end, char32_t c) {
  ASSERT(c <= 0xFF);

  unsigned len = Utf8::TrailLengths[c];
  ASSERT(len <= 3);

  if (UNLIKELY(it + len > end)) {
    SkipWtf8Trail(it, end);
    return EndOfStreamError;
  }

  const char* start = it;

  unsigned illegal;
  switch (len) {
    case 3:
      c &= 0x07;

      illegal = DecodeOneInWtf8Sequence(it, c);
      if (c >= 0x110)
        illegal = 1;
      illegal |= DecodeOneInWtf8Sequence(it, c);
      illegal |= DecodeOneInWtf8Sequence(it, c);
      if (illegal || c < 0x10000) {
        it = start;
        SkipWtf8Trail(it, end);
        return InvalidSequenceError;
      }
      return c;

    case 2: // may include unpaired surrogate
      c &= 0x0F;
      illegal = DecodeOneInWtf8Sequence(it, c);
      illegal |= DecodeOneInWtf8Sequence(it, c);
      if (illegal || c < 0x800) {
        it = start;
        SkipWtf8Trail(it, end);
        return InvalidSequenceError;
      }
      return c;

    case 1:
      c &= 0x1F;
      illegal = DecodeOneInWtf8Sequence(it, c);
      if (illegal || c < 0x80) {
        it = start;
        SkipWtf8Trail(it, end);
        return InvalidSequenceError;
      }
      return c;

    case 0:
      return InvalidSequenceError;
  }
  UNREACHABLE(return InvalidSequenceError);
}

char32_t Wtf16::DecodeSlow(const char16_t*& it, const char16_t* end, char32_t lead) {
  ASSERT(lead <= MaxCodepoint);

  // check for end of stream.
  if (it >= end)
    return lead;

  if (!SurrogateIsLeading(lead))
    return lead; // unpaired trail surrogate

  char16_t trail = *it++;

  if (!IsTrailSurrogate(trail)) {
    // unpaired surrogate
    --it;
    return lead;
  }
  // valid surrogate pair
  char32_t decoded = DecodeSurrogatePair(lead, trail);
  ASSERT(IsValidCodepoint(decoded));
  return decoded;
}

template<typename T>
static inline void WriteWtfTmpl(TextWriter& out, Span<T> wtf) {
  const T* it = begin(wtf);
  const T* it_end = end(wtf);

  while (it < it_end) {
    const T* valid_begin = it;
    const T* valid_end = it;

    do {
      char32_t c = UtfTmpl<T>::Decode(it, it_end);
      if (UtfTmpl<T>::IsDecodeError(c))
        break;
      valid_end = it;
    } while (it < it_end);

    if (valid_begin != valid_end)
      out.Write(MakeSpan(valid_begin, valid_end - valid_begin));

    if (valid_end != it_end)
      out.Write(Unicode::ReplacementCharacter);
  }
}

void WriteWtf(TextWriter& out, StringSpan wtf) { WriteWtfTmpl(out, wtf); }
void WriteWtf(TextWriter& out, String16Span wtf) { WriteWtfTmpl(out, wtf); }

template<typename T>
static inline String WtfToUtf8Tmpl(Span<T> wtf) {
  String utf;
  utf.EnsureCapacity(wtf.size());

  StringWriter writer(&utf);
  WriteWtf(writer, wtf);
  return utf;
}

String WtfToUtf8(StringSpan wtf) { return WtfToUtf8Tmpl(wtf); }
String WtfToUtf8(String16Span wtf) { return WtfToUtf8Tmpl(wtf); }

template<typename TOutput, typename TInput>
static inline void AppendWtfTmpl(List<TOutput>& output, Span<TInput> wtf) {
  if constexpr (sizeof(TOutput) == sizeof(TInput)) {
    output.EnsureCapacity(output.size() + wtf.size());
  }
  StringWriter writer(&output);
  WriteWtf(writer, wtf);
}

void AppendWtf(String& output, StringSpan wtf) { AppendWtfTmpl(output, wtf); }
void AppendWtf(String& output, String16Span wtf) { AppendWtfTmpl(output, wtf); }

} // namespace stp
