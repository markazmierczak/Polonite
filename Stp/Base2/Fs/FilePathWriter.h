// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_FILEPATHWRITER_H_
#define STP_BASE_FS_FILEPATHWRITER_H_

#include "Base/Fs/FilePath.h"
#include "Base/Io/TextWriter.h"

namespace stp {

class BASE_EXPORT FilePathWriter final : public TextWriter {
 public:
  explicit FilePathWriter(FilePath& path) : path_(path) {}

  void EnsureSeparator();

  const TextCodec& GetEncoding() const override;

 protected:
  void OnWriteAsciiChar(char c) override;
  void OnWriteUnicodeChar(char32_t c) override;
  void OnWriteAscii(StringSpan text) override;
  void OnWriteUtf8(StringSpan text) override;
  void OnWriteUtf16(String16Span text) override;
  void OnWriteEncoded(const BufferSpan& text, const TextCodec& encoding) override;
  void OnEndLine() override;

 private:
  FilePath& path_;
};

} // namespace stp

#endif // STP_BASE_FS_FILEPATHWRITER_H_
