// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_CODEC_TEXTCODEC_H_
#define STP_BASE_TEXT_CODEC_TEXTCODEC_H_

#include "Base/Containers/BufferSpan.h"
#include "Base/Error/Exception.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/StringSpan.h"
#include "Base/Type/HashableFwd.h"

namespace stp {

class TextConversionContext {};

struct TextConversionResult {
  TextConversionResult(int num_read, int num_wrote, bool did_fallback)
      : num_read(num_read), num_wrote(num_wrote), did_fallback(did_fallback) {}

  int num_read;
  int num_wrote;
  bool did_fallback;
};

class BASE_EXPORT TextConversionFallbackException final : public Exception {
 public:
  StringSpan GetName() const noexcept override;
};

struct TextCodecVtable {
  TextConversionResult (*decode)(TextConversionContext*, BufferSpan, MutableStringSpan, bool);
  TextConversionResult (*decode16)(TextConversionContext*, BufferSpan, MutableString16Span, bool);
  TextConversionResult (*encode)(TextConversionContext*, StringSpan, MutableBufferSpan);
  TextConversionResult (*encode16)(TextConversionContext*, String16Span, MutableBufferSpan);
};

struct TextCodec {
  const TextCodecVtable* vtable = nullptr;

  StringSpan name;
  Span<StringSpan> aliases;

  int iana_codepage = 0;
  int windows_codepage = 0;

  int context_size = 0;

  bool single_byte = false;
};

namespace detail {

BASE_EXPORT extern const TextCodec UndefinedTextCodec;
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

} // namespace detail

class BASE_EXPORT TextEncoding {
 public:
  constexpr TextEncoding() noexcept : codec_(detail::UndefinedTextCodec) {}
  constexpr TextEncoding(const TextCodec* codec) noexcept : codec_(*codec) {}

  // Name and aliases are specified in IANA character sets:
  // https://www.iana.org/assignments/character-sets/character-sets.xhtml
  // |name| is standard IANA name for character set.
  StringSpan GetName() const { return codec_.name; }
  Span<StringSpan> GetAliases() const { return codec_.aliases; }

  int GetIanaCodepage() { return codec_.iana_codepage; }
  int GetWindowsCodepage() const { return codec_.windows_codepage; }

  bool CanDecode() const { return codec_.vtable->decode != nullptr; }
  bool CanEncode() const { return codec_.vtable->encode != nullptr; }

  // True if each unit takes single byte and ASCII compatible.
  bool IsSingleByte() const { return codec_.single_byte; }

  bool IsValid() const { return &codec_ != &detail::UndefinedTextCodec; }

  const TextCodecVtable& GetVtable() const { return *codec_.vtable; }

  friend bool operator==(const TextEncoding& l, const TextEncoding& r) { return &l == &r; }
  friend bool operator!=(const TextEncoding& l, const TextEncoding& r) { return !operator==(l, r); }
  friend HashCode Hash(const TextEncoding& codec) { return codec.HashImpl(); }

  static bool AreNamesMatching(StringSpan lhs, StringSpan rhs) noexcept;

  static TextEncoding BuiltinAscii() { return &detail::AsciiCodec; }
  static TextEncoding BuiltinCp1252() { return &detail::Cp1252Codec; }
  static TextEncoding BuiltinLatin1() { return &detail::Latin1Codec; }
  static TextEncoding BuiltinLatin2() { return &detail::Latin2Codec; }
  static TextEncoding BuiltinLatin3() { return &detail::Latin3Codec; }
  static TextEncoding BuiltinLatin4() { return &detail::Latin4Codec; }

  static TextEncoding BuiltinUtf8() { return &detail::Utf8Codec; }
  static TextEncoding BuiltinUtf16() { return &detail::Utf16Codec; }
  static TextEncoding BuiltinUtf16BE() { return &detail::Utf16BECodec; }
  static TextEncoding BuiltinUtf16LE() { return &detail::Utf16LECodec; }
  static TextEncoding BuiltinUtf32() { return &detail::Utf32Codec; }
  static TextEncoding BuiltinUtf32BE() { return &detail::Utf32BECodec; }
  static TextEncoding BuiltinUtf32LE() { return &detail::Utf32LECodec; }

 private:
  const TextCodec& codec_;

  HashCode HashImpl() const noexcept;
};

BASE_EXPORT String ToString(BufferSpan buffer, TextEncoding codec);
BASE_EXPORT String16 ToString16(BufferSpan buffer, TextEncoding codec);

class TextCodecBuilder {
 public:
  constexpr TextCodecBuilder() = default;

  constexpr TextCodecBuilder(StringSpan name, const TextCodecVtable& vtable) {
    codec_.name = name;
    codec_.vtable = &vtable;
  }

  constexpr operator TextCodec() const { return codec_; }

  constexpr void SetAliases(Span<StringSpan> aliases) { codec_.aliases = aliases; }
  constexpr void SetIanaCodepage(int cp) { codec_.iana_codepage = cp; }
  constexpr void SetWindowsCodepage(int cp) { codec_.windows_codepage = cp; }

 private:
  TextCodec codec_;
};

constexpr TextCodecBuilder BuildTextCodec(StringSpan name, const TextCodecVtable& vtable) {
  return TextCodecBuilder(name, vtable);
}

} // namespace stp

#endif // STP_BASE_TEXT_CODEC_TEXTCODEC_H_
