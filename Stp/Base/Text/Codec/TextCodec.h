// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_CODEC_TEXTCODEC_H_
#define STP_BASE_TEXT_CODEC_TEXTCODEC_H_

#include "Base/Containers/BufferSpan.h"
#include "Base/Error/Exception.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/Codec/TextCodecFwd.h"
#include "Base/Text/StringSpan.h"
#include "Base/Text/Utf.h"
#include "Base/Type/HashableFwd.h"

namespace stp {

struct TextConversionContext {
  union State {
    State() { memset(this, 0, sizeof(TextConversionContext)); }
    void* ptr;
    byte_t bytes[8];
  };
  State state;
  bool exception_on_fallback = false;

  void MaybeThrow(bool saw_error);
};

class BASE_EXPORT TextConversionFallbackException final : public Exception {
 public:
  StringSpan GetName() const noexcept override;
};

struct TextCodecVtable {
  using Context = TextConversionContext;

  int (*decode)(Context&, BufferSpan, MutableStringSpan, bool);
  int (*decode16)(Context&, BufferSpan, MutableString16Span, bool);
  int (*count_chars)(const Context&, BufferSpan);
  int (*count_chars16)(const Context&, BufferSpan);

  int (*encode)(Context&, StringSpan, MutableBufferSpan);
  int (*encode16)(Context&, String16Span, MutableBufferSpan);
  int (*count_bytes)(const Context&, StringSpan);
  int (*count_bytes16)(const Context&, String16Span);
};

namespace detail {

struct TextCodecData {
  const TextCodecVtable* vtable = nullptr;

  StringSpan name;
  Span<StringSpan> aliases;

  int iana_codepage = 0;
  int windows_codepage = 0;

  bool single_byte = false;
};

} // namespace detail

class BASE_EXPORT TextCodec {
 public:
  constexpr explicit TextCodec(detail::TextCodecData d) : d_(d) {}

  // Name and aliases are specified in IANA character sets:
  // https://www.iana.org/assignments/character-sets/character-sets.xhtml
  // |name| is standard IANA name for character set.
  StringSpan GetName() const { return d_.name; }
  Span<StringSpan> GetAliases() const { return d_.aliases; }

  int GetIanaCodepage() { return d_.iana_codepage; }
  int GetWindowsCodepage() const { return d_.windows_codepage; }

  bool CanDecode() const { return d_.vtable->decode != nullptr; }
  bool CanEncode() const { return d_.vtable->encode != nullptr; }

  // True if each unit takes single byte and ASCII compatible.
  bool IsSingleByte() const { return d_.single_byte; }

  const TextCodecVtable& GetVtable() const { return *d_.vtable; }

  friend bool operator==(const TextCodec& l, const TextCodec& r) { return &l == &r; }
  friend bool operator!=(const TextCodec& l, const TextCodec& r) { return !operator==(l, r); }
  friend HashCode Hash(const TextCodec& codec) { return codec.HashImpl(); }

 private:
  detail::TextCodecData d_;

  HashCode HashImpl() const;

  DISALLOW_COPY_AND_ASSIGN(TextCodec);
};

BASE_EXPORT String ToString(BufferSpan buffer, const TextCodec& codec);
BASE_EXPORT String16 ToString16(BufferSpan buffer, const TextCodec& codec);

class TextCodecBuilder {
 public:
  constexpr TextCodecBuilder() = default;

  constexpr TextCodecBuilder(StringSpan name, const TextCodecVtable& vtable) {
    d_.name = name;
    d_.vtable = &vtable;
  }

  constexpr operator TextCodec() const { return TextCodec(d_); }

  constexpr void SetAliases(Span<StringSpan> aliases) { d_.aliases = aliases; }
  constexpr void SetIanaCodepage(int cp) { d_.iana_codepage = cp; }
  constexpr void SetWindowsCodepage(int cp) { d_.windows_codepage = cp; }

 private:
  detail::TextCodecData d_;
};

constexpr TextCodecBuilder BuildTextCodec(StringSpan name, const TextCodecVtable& vtable) {
  return TextCodecBuilder(name, vtable);
}

inline void TextConversionContext::MaybeThrow(bool saw_error) {
  if (saw_error && exception_on_fallback)
    throw TextConversionFallbackException();
}

} // namespace stp

#endif // STP_BASE_TEXT_CODEC_TEXTCODEC_H_
