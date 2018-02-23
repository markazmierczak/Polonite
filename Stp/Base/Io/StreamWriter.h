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

  void forceValidation();

  int getBufferCapacity() const { return buffer_.capacity(); }

  void setAutoFlush(bool auto_flush);
  bool getAutoFlush() const { return auto_flush_; }

  Stream& getStream() const { return stream_; }
  TextEncoding getEncoding() const override;

 protected:
  void onWriteChar(char c) override;
  void onWriteRune(char32_t c) override;
  void onWriteString(StringSpan text) override;
  void onFlush() override;

 private:
  Stream& stream_;

  Buffer buffer_;

  TextEncoding encoding_;
  TextEncoder* encoder_ = nullptr;

  bool auto_flush_ = false;

  Buffer encoder_memory_;

  static constexpr int MinBufferCapacity = 1024;
  static constexpr int MaxLengthForFastPath_ = 8;

  bool isDirect() const { return encoder_ == nullptr; }
  void writeIndirect(StringSpan input);

  void writeToBuffer(BufferSpan input);
  void flushBuffer();

  void createEncoder();
  void destroyEncoder();
};

} // namespace stp

#endif // STP_BASE_IO_STREAMWRITER_H_
