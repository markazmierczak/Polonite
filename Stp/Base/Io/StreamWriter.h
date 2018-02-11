// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_STREAMWRITER_H_
#define STP_BASE_IO_STREAMWRITER_H_

#include "Base/Containers/Buffer.h"
#include "Base/Io/Stream.h"
#include "Base/Io/TextWriter.h"
#include "Base/Text/TextEncoder.h"

namespace stp {

class BASE_EXPORT StreamWriter final : public TextWriter {
 public:
  explicit StreamWriter(Stream* stream);
  explicit StreamWriter(Stream* stream, TextEncoding encoding);
  explicit StreamWriter(Stream* stream, TextEncoding encoding, int buffer_capacity);
  ~StreamWriter();

  ALWAYS_INLINE Stream* stream() const { return stream_; }

  int GetBufferCapacity() const { return buffer_.capacity(); }

  void SetAutoFlush(bool auto_flush);
  bool GetAutoFlush() const { return auto_flush_; }

  TextEncoding GetEncoding() const override;

 protected:
  void OnWriteAsciiChar(char c) override;
  void OnWriteUnicodeChar(char32_t c) override;
  void OnWriteAscii(StringSpan text) override;
  void OnWriteUtf8(StringSpan text) override;
  void OnWriteUtf16(String16Span text) override;
  void OnFlush() override;

 private:
  enum class CodecClass : unsigned char {
    Utf8,
    Utf16,
    None,
  };

  Stream* stream_;
  Buffer buffer_;

  TextEncoding encoding_;
  TextEncoder encoder_;

  CodecClass codec_class_;
  bool auto_flush_ = false;

  static constexpr int MinBufferCapacity = 1024;
  static constexpr int MaxLengthForFastPath_ = 8;

  void WriteToBuffer(BufferSpan input);
  void FlushBuffer();

  void WriteUnicodeCharAsUtf8(char32_t codepoint);
  void WriteUnicodeCharAsUtf16(char32_t codepoint);

  template<typename TDst, typename TSrc>
  void OnWriteAsciiTmpl(const TSrc* text, int length);

  template<typename TDst, typename TSrc>
  void ConvertUnicode(const TSrc* text, int length);
  template<typename TDst, typename TSrc>
  void ConvertAsciiOrUnicode(const TSrc* text, int length);

  static CodecClass SelectCodecClass(TextEncoding encoding);
};

} // namespace stp

#endif // STP_BASE_IO_STREAMWRITER_H_
