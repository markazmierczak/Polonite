// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_INLINESTRINGWRITER_H_
#define STP_BASE_IO_INLINESTRINGWRITER_H_

#include "Base/Containers/InlineList.h"
#include "Base/Io/TextWriter.h"

namespace stp {

class BASE_EXPORT InlineStringWriter final : public TextWriter {
 public:
  explicit InlineStringWriter(InlineListBase<char>& string) : string_(string) {}

  const TextCodec& GetEncoding() const override;

 protected:
  void OnWriteAsciiChar(char c) override;
  void OnWriteUnicodeChar(char32_t c) override;
  void OnWriteAscii(StringSpan text) override;
  void OnWriteUtf8(StringSpan text) override;
  void OnWriteUtf16(String16Span text) override;
  void OnWriteEncoded(const BufferSpan& text, const TextCodec& encoding) override;
  void OnIndent(int count, char c) override;

 private:
  InlineListBase<char>& string_;
};

class BASE_EXPORT InlineString16Writer final : public TextWriter {
 public:
  explicit InlineString16Writer(InlineListBase<char16_t>& string) : string_(string) {}

  const TextCodec& GetEncoding() const override;

 protected:
  void OnWriteAsciiChar(char c) override;
  void OnWriteUnicodeChar(char32_t c) override;
  void OnWriteAscii(StringSpan text) override;
  void OnWriteUtf8(StringSpan text) override;
  void OnWriteUtf16(String16Span text) override;
  void OnWriteEncoded(const BufferSpan& text, const TextCodec& encoding) override;
  void OnIndent(int count, char c) override;

 private:
  InlineListBase<char16_t>& string_;
};

namespace detail {

template<typename T>
struct InlineStringTmplWriterTmpl;

template<> struct InlineStringTmplWriterTmpl<char> { typedef InlineStringWriter Type; };
template<> struct InlineStringTmplWriterTmpl<char16_t> { typedef InlineString16Writer Type; };

} // namespace detail

template<typename T>
using InlineStringTmplWriter = typename detail::InlineStringTmplWriterTmpl<T>::Type;

} // namespace stp

#endif // STP_BASE_IO_INLINESTRINGWRITER_H_
