// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/FilePathWriter.h"

#include "Base/Debug/Log.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/StringUtfConversions.h"
#include "Base/Text/TextEncoding.h"
#include "Base/Text/Wtf.h"

namespace stp {

TextEncoding FilePathWriter::GetEncoding() const {
  #if HAVE_UTF8_NATIVE_VALIDATION
  return &Utf8Codec;
  #else
  // Encoding is unknown for paths for most systems,
  // Linux and Windows does not validate them.
  // See also documentation for FilePath for more information.
  return TextEncoding();
  #endif
}

void FilePathWriter::EnsureSeparator() {
  if (path_.IsEmpty())
    return;
  if (IsFilePathSeparator(path_.chars().GetLast()))
    return;
  path_.chars().Add(FilePathSeparator);
}

void FilePathWriter::OnWriteChar(char c) {
  path_.chars().Add(char_cast<FilePathChar>(c));
}

void FilePathWriter::OnWriteRune(char32_t codepoint) {
  AppendUnicodeCharacter(path_.chars(), codepoint);
}

void FilePathWriter::OnWriteString(StringSpan text) {
  if (!AppendUnicode(path_.chars(), text))
    LOG(WARN, "ignored some illegal sequences from input to file path");
}

void FilePathWriter::OnEndLine() {
  // unable to add new line to FilePath
  throw NotSupportedException();
}

} // namespace stp
