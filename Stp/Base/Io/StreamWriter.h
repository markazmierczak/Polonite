// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_STREAMWRITER_H_
#define STP_BASE_IO_STREAMWRITER_H_

#include "Base/Io/Stream.h"
#include "Base/Io/TextWriter.h"
#include "Base/Text/Codec/TextEncoder.h"

namespace stp {

class BASE_EXPORT StreamWriter final : public TextWriter {
 public:
  explicit StreamWriter(Stream* stream);
  explicit StreamWriter(Stream* stream, TextCodec codec);
  explicit StreamWriter(Stream* stream, TextCodec codec, int buffer_capacity);
  ~StreamWriter();

  ALWAYS_INLINE Stream* stream() const { return stream_; }

  int GetBufferCapacity() const { return buffer_.capacity(); }

  void SetAutoFlush(bool auto_flush);
  bool GetAutoFlush() const { return auto_flush_; }

  TextCodec GetEncoding() const override;

 protected:
  void OnWriteAsciiChar(char c) override;
  void OnWriteUnicodeChar(char32_t c) override;
  void OnWriteAscii(StringSpan text) override;
  void OnWriteUtf8(StringSpan text) override;
  void OnWriteUtf16(String16Span text) override;
  void OnWriteBytes(const BufferSpan& text) override;
  void OnWriteCustom(const BufferSpan& text, TextCodec encoding) override;
  void OnFlush() override;

 private:
  enum class CodecClass : unsigned char {
    Utf8,
    Utf16,
    Utf32,
    AsciiCompatbile,
    None,
  };

  Stream* stream_;
  Buffer buffer_;

  TextEncoder encoder_;
  TextDecoder decoder_;

  CodecClass codec_class_;
  bool auto_flush_ = false;
  // TODO use to handle fallbacks
  // UnencodableHandling unencodable_handling_ = UnencodableHandling::Replace;
  // UndecodableHandling undecodable_handling_ = UndecodableHandling::Replace;

  static constexpr int MinBufferCapacity = 1024;
  static constexpr int MaxLengthForFastPath_ = 8;

  void WriteToStream(const byte_t* bytes, int size);
  void WriteBytesInternal(const byte_t* bytes, int size);
  void FlushBuffer();

  void WriteUnicodeCharAsUtf8(char32_t codepoint);
  void WriteUnicodeCharAsUtf16(char32_t codepoint);

  template<typename TDst, typename TSrc>
  void OnWriteAsciiTmpl(const TSrc* text, int length);

  template<typename TDst, typename TSrc>
  void ConvertUnicode(const TSrc* text, int length);
  template<typename TDst, typename TSrc>
  void ConvertAsciiOrUnicode(const TSrc* text, int length);

  // nullptr in |encoder_.context| means TextWriter is not using transcoder.
  // This is done to optimize things for recommended String type.
  // Transcoder is initialized automatically when needed.
  bool IsTranscoderInitialized() const { return encoder_.context != nullptr; }

  void InitTranscoder();

  void SwitchDecoder(TextCodec codec);

  void Convert(const byte_t* bytes, int size, TextCodec encoding);

  void WriteWithEncoder(const char32_t* characters, int length);

  void BeginWrite();
  void EndWrite();

  static CodecClass SelectCodecClass(TextCodec codec);
};

} // namespace stp

#endif // STP_BASE_IO_STREAMWRITER_H_
