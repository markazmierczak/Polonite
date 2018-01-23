// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Codec/TextCodecDatabase.h"

#include "Base/Text/AsciiChar.h"
#include "Base/Util/LazyInstance.h"

namespace stp {

static inline StringSpan RemoveNonAlphaNumericPrefix(StringSpan s) {
  while (!s.IsEmpty() && !IsAlphaNumericAscii(s.GetFirst()))
    s.RemovePrefix(1);
  return s;
}

static bool TextCodecNamesMatch(StringSpan lhs, StringSpan rhs) {
  if (lhs == rhs)
    return true;

  while (!lhs.IsEmpty() && !rhs.IsEmpty()) {
    lhs = RemoveNonAlphaNumericPrefix(lhs);
    rhs = RemoveNonAlphaNumericPrefix(rhs);
    if (lhs.IsEmpty() || rhs.IsEmpty())
      break;

    if (ToUpperAscii(lhs.GetFirst()) != ToUpperAscii(rhs.GetFirst()))
      return false;

    lhs.RemovePrefix(1);
    rhs.RemovePrefix(1);
  }
  return lhs.IsEmpty() && rhs.IsEmpty();
}

static LazyInstance<TextCodecDatabase>::LeakAtExit g_codec_database;

static const TextCodec* const BuiltinCodecs[] = {
  // Sort this array by frequency of usage.
  &Utf8Codec,
  &Utf16Codec,
  &Utf16BECodec,
  &Utf16LECodec,
  &Utf32Codec,
  &Utf32BECodec,
  &Utf32LECodec,
  &AsciiCodec,
  &Cp1252Codec,
  &Latin1Codec,
  &Latin2Codec,
  &Latin3Codec,
  &Latin4Codec,
};

// TODO introduce providers
// TODO use FlatMap with case insensitive comparison
TextCodec* TextCodecDatabase::GetForName(StringSpan name) {
  TextCodecDatabase& database = *g_codec_database;

  auto codec = database.GetForName(name);
  if (codec.IsNull()) {
    for (auto* codec : BuiltinCodecs) {
      if (TextCodecNamesMatch(codec->GetName(), name)) {
        codec = TextCodec(it);
        break;
      }
      for (auto& alias : it->aliases) {
        if (TextCodecNamesMatch(alias, name)) {
          codec = TextCodec(it);
          break;
        }
      }
      if (!codec.IsNull())
        break;
    }
    // Write the result even codec is not found.
    // Next search will return immediately.
    database.SetForName(name, codec);
  }
  return codec;
}

} // namespace stp
