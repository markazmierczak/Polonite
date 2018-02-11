// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_TEXTENCODINGDATABUILDER_H_
#define STP_BASE_TEXT_TEXTENCODINGDATABUILDER_H_

#include "Base/Memory/PolymorphicAllocator.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/TextEncoding.h"

namespace stp {

class TextEncodingDataBuilder {
 public:
  constexpr TextEncodingDataBuilder() = default;

  constexpr TextEncodingDataBuilder(
      StringSpan name,
      TextDecoderFactory create_decoder, TextEncoderFactory create_encoder) {
    d_.name = name;
    d_.create_decoder = create_decoder;
    d_.create_encoder = create_encoder;
  }

  constexpr operator TextEncodingData() const { return d_; }

  constexpr void SetConfig(const TextEncodingConfig* config) { d_.config = config; }

 private:
  TextEncodingData d_;
};

namespace detail {

template<typename TDecoder>
inline TextDecoder* MakeTextDecoder(
    PolymorphicAllocator& allocator, const TextEncodingConfig* config) {
  return new(allocator.Allocate(isizeof(TDecoder))) TDecoder(config);
}

template<typename TEncoder>
inline TextEncoder* MakeTextEncoder(
    PolymorphicAllocator& allocator, const TextEncodingConfig* config) {
  return new(allocator.Allocate(isizeof(TEncoder))) TEncoder(config);
}

} // namespace detail

template<typename TDecoder, typename TEncoder>
constexpr TextEncodingDataBuilder BuildTextEncodingData(StringSpan name) {
  return TextEncodingDataBuilder(
      name, &detail::MakeTextDecoder<TDecoder>, &detail::MakeTextEncoder<TEncoder>);
}

} // namespace stp

#endif // STP_BASE_TEXT_TEXTENCODINGDATABUILDER_H_
