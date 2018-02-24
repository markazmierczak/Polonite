// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_CODEC_TEXTCODEC_H_
#define STP_BASE_TEXT_CODEC_TEXTCODEC_H_

#include "Base/Containers/BufferSpan.h"
#include "Base/Containers/Span.h"

namespace stp {

class PolymorphicAllocator;

namespace detail {

struct TextRecodeResult {
  TextRecodeResult(int num_read, int num_wrote, bool more_output)
      : num_read(num_read), num_wrote(num_wrote), more_output(more_output) {}

  // Number of input/output units processed when decoding/encoding text.
  int num_read;
  int num_wrote;
  // True when encoder/decoder requests larger output buffer.
  bool more_output;
};

} // namespace detail

class TextDecoder {
 public:
  using Result = detail::TextRecodeResult;

  virtual ~TextDecoder() {}
  virtual Result Decode(BufferSpan input, MutableStringSpan output, bool flush) = 0;
};

class TextEncoder {
 public:
  using Result = detail::TextRecodeResult;

  virtual ~TextEncoder() {}
  virtual Result Encode(StringSpan input, MutableBufferSpan output) = 0;
};

struct TextEncodingConfig {};

typedef TextDecoder* (*TextDecoderFactory)(PolymorphicAllocator& allocator, const TextEncodingConfig* config);
typedef TextEncoder* (*TextEncoderFactory)(PolymorphicAllocator& allocator, const TextEncodingConfig* config);

struct TextEncodingData {
  TextDecoderFactory create_decoder = nullptr;
  TextEncoderFactory create_encoder = nullptr;

  StringSpan name;

  const TextEncodingConfig* config = nullptr;
};

namespace detail {
BASE_EXPORT extern const TextEncodingData UndefinedTextEncodingData;
}

class BASE_EXPORT TextEncoding {
 public:
  constexpr TextEncoding() noexcept : codec_(detail::UndefinedTextEncodingData) {}
  constexpr TextEncoding(const TextEncodingData* codec) noexcept : codec_(*codec) {}

  // The name as specified in IANA character sets:
  // https://www.iana.org/assignments/character-sets/character-sets.xhtml
  StringSpan GetName() const { return codec_.name; }

  bool CanDecode() const { return codec_.create_decoder != nullptr; }
  bool CanEncode() const { return codec_.create_encoder != nullptr; }

  TextDecoder* CreateDecoder(PolymorphicAllocator& allocator);
  TextEncoder* CreateEncoder(PolymorphicAllocator& allocator);

  bool IsValid() const { return &codec_ != &detail::UndefinedTextEncodingData; }

  friend bool operator==(const TextEncoding& l, const TextEncoding& r) { return &l == &r; }
  friend bool operator!=(const TextEncoding& l, const TextEncoding& r) { return !operator==(l, r); }
  friend HashCode partialHash(const TextEncoding& codec) { return codec.HashImpl(); }

  static bool AreNamesMatching(StringSpan lhs, StringSpan rhs) noexcept;

 private:
  const TextEncodingData& codec_;

  HashCode HashImpl() const noexcept;
};

BASE_EXPORT String ToString(BufferSpan buffer, TextEncoding codec);

inline TextDecoder* TextEncoding::CreateDecoder(PolymorphicAllocator& allocator) {
  ASSERT(CanDecode());
  return (*codec_.create_decoder)(allocator, codec_.config);
}

inline TextEncoder* TextEncoding::CreateEncoder(PolymorphicAllocator& allocator) {
  ASSERT(CanEncode());
  return (*codec_.create_encoder)(allocator, codec_.config);
}

} // namespace stp

#endif // STP_BASE_TEXT_CODEC_TEXTCODEC_H_
