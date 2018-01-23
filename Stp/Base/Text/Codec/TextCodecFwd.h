// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_CODEC_TEXTCODECFWD_H_
#define STP_BASE_TEXT_CODEC_TEXTCODECFWD_H_

#include "Base/Export.h"

namespace stp {

class TextCodec;

BASE_EXPORT extern const TextCodec AsciiCodec;
BASE_EXPORT extern const TextCodec Cp1252Codec;
BASE_EXPORT extern const TextCodec Latin1Codec;
BASE_EXPORT extern const TextCodec Latin2Codec;
BASE_EXPORT extern const TextCodec Latin3Codec;
BASE_EXPORT extern const TextCodec Latin4Codec;
BASE_EXPORT extern const TextCodec Utf16BECodec;
BASE_EXPORT extern const TextCodec Utf16LECodec;
BASE_EXPORT extern const TextCodec Utf16Codec;
BASE_EXPORT extern const TextCodec Utf32BECodec;
BASE_EXPORT extern const TextCodec Utf32LECodec;
BASE_EXPORT extern const TextCodec Utf32Codec;
BASE_EXPORT extern const TextCodec Utf8Codec;

} // namespace stp

#endif // STP_BASE_TEXT_CODEC_TEXTCODECFWD_H_
