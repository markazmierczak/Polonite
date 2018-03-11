// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/FilePathWriter.h"

#include "Base/Text/AsciiChar.h"
#include "Base/Text/TextEncoding.h"
#include "Base/Text/Wtf.h"

namespace stp {

TextEncoding FilePathWriter::getEncoding() const {
  #if HAVE_UTF8_NATIVE_VALIDATION
  return &Utf8Codec;
  #else
  // Encoding is unknown for paths for most systems,
  // Linux and Windows does not validate them.
  // See also documentation for FilePath for more information.
  return TextEncoding();
  #endif
}

void FilePathWriter::ensureSeparator() {
  if (path_.isEmpty())
    return;
  if (isFilePathSeparator(path_.chars().last()))
    return;
  path_.chars().add(FilePathSeparator);
}

void FilePathWriter::onWriteChar(char c) {
  path_.chars().add(charCast<FilePathChar>(c));
}

void FilePathWriter::onWriteRune(char32_t rune) {
  appendRune(path_.chars(), rune);
}

void FilePathWriter::onWriteString(StringSpan text) {
  #if OS(WIN)
  #error "not implemented"
  #elif OS(POSIX)
  path_.chars().append(text);
  #endif
}

void FilePathWriter::onEndLine() {
  // unable to add new line to FilePath
  throw NotSupportedException();
}

} // namespace stp
