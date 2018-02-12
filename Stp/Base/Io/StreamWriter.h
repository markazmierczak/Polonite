// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_STREAMWRITER_H_
#define STP_BASE_IO_STREAMWRITER_H_

#include "Base/Containers/Buffer.h"
#include "Base/Io/TextWriter.h"
#include "Base/Text/TextEncoding.h"

namespace stp {

class Stream;

class BASE_EXPORT StreamWriter final : public TextWriter {
 public:
  explicit StreamWriter(Stream* stream);
  explicit StreamWriter(Stream* stream, TextEncoding encoding);
  explicit StreamWriter(Stream* stream, TextEncoding encoding, int buffer_capacity);
  ~StreamWriter();

  void ForceValidation();

  int GetBufferCapacity() const { return buffer_.capacity(); }

  void SetAutoFlush(bool auto_flush);
  bool GetAutoFlush() const { return auto_flush_; }

  Stream& GetStream() const { return stream_; }
  TextEncoding GetEncoding() const override;

 protected:
  void OnWriteChar(char c) override;
  void OnWriteRune(char32_t c) override;
  void OnWriteString(StringSpan text) override;
  void OnFlush() override;

 private:
  Stream& stream_;

  Buffer buffer_;

  TextEncoding encoding_;
  TextEncoder* encoder_ = nullptr;

  bool auto_flush_ = false;

  Buffer encoder_memory_;

  static constexpr int MinBufferCapacity = 1024;
  static constexpr int MaxLengthForFastPath_ = 8;

  bool IsDirect() const { return encoder_ == nullptr; }
  void WriteIndirect(StringSpan input);

  void WriteToBuffer(BufferSpan input);
  void FlushBuffer();

  void CreateEncoder();
  void DestroyEncoder();
};

} // namespace stp

#endif // STP_BASE_IO_STREAMWRITER_H_
