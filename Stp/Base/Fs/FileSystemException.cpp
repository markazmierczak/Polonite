// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Fs/FileSystemException.h"

#include "Base/Posix/PosixErrorCode.h"
#include "Base/Io/TextWriter.h"

namespace stp {

StringSpan FileSystemException::GetName() const noexcept {
  return "FileSystemException";
}

void FileSystemException::OnFormat(TextWriter& out) const {
  out << GetErrorCode();
  if (!path_.IsEmpty()) {
    out.WriteAscii(", path=");
    out << path_;
  }
  if (!aux_path_.IsEmpty()) {
    out.WriteAscii(", aux_path=");
    out << aux_path_;
  }
}

} // namespace stp
