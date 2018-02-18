// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_FILEPATHWRITER_H_
#define STP_BASE_FS_FILEPATHWRITER_H_

#include "Base/FileSystem/FilePath.h"
#include "Base/Io/TextWriter.h"

namespace stp {

class BASE_EXPORT FilePathWriter final : public TextWriter {
 public:
  explicit FilePathWriter(FilePath& path) : path_(path) {}

  void EnsureSeparator();

  TextEncoding GetEncoding() const override;

 protected:
  void onWriteChar(char c) override;
  void onWriteRune(char32_t c) override;
  void onWriteString(StringSpan text) override;
  void onEndLine() override;

 private:
  FilePath& path_;
};

} // namespace stp

#endif // STP_BASE_FS_FILEPATHWRITER_H_
