// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_STRINGWRITER_H_
#define STP_BASE_IO_STRINGWRITER_H_

#include "Base/Containers/List.h"
#include "Base/Io/TextWriter.h"

namespace stp {

class BASE_EXPORT StringWriter final : public TextWriter {
 public:
  explicit StringWriter(String* string) : string_(*string) {}

  TextEncoding GetEncoding() const override;

 protected:
  void OnWriteAsciiChar(char c) override;
  void OnWriteUnicodeChar(char32_t c) override;
  void OnWriteAscii(StringSpan text) override;
  void OnWriteUtf8(StringSpan text) override;
  void OnWriteUtf16(String16Span text) override;
  void OnIndent(int count, char c) override;

 private:
  String& string_;
};

class BASE_EXPORT String16Writer : public TextWriter {
 public:
  explicit String16Writer(String16* string) : string_(*string) {}

  TextEncoding GetEncoding() const override;

 protected:
  void OnWriteAsciiChar(char c) override;
  void OnWriteUnicodeChar(char32_t c) override;
  void OnWriteAscii(StringSpan text) override;
  void OnWriteUtf8(StringSpan text) override;
  void OnWriteUtf16(String16Span text) override;
  void OnIndent(int count, char c) override;

 private:
  String16& string_;
};

namespace detail {

template<typename T>
struct StringTmplWriterTmpl;

template<> struct StringTmplWriterTmpl<char> { typedef StringWriter Type; };
template<> struct StringTmplWriterTmpl<char16_t> { typedef String16Writer Type; };

} // namespace detail

template<typename T>
using StringTmplWriter = typename detail::StringTmplWriterTmpl<T>::Type;

} // namespace stp

#endif // STP_BASE_IO_STRINGWRITER_H_
