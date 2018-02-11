// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/FileSystemException.h"

#include "Base/Posix/PosixErrorCode.h"
#include "Base/Type/Formattable.h"

namespace stp {

StringSpan FileSystemException::GetName() const noexcept {
  return "FileSystemException";
}

void FileSystemException::OnFormat(TextWriter& out) const {
  out << GetErrorCode();
  if (!path_.IsEmpty()) {
    out << ", path=" << path_;
  }
  if (!aux_path_.IsEmpty()) {
    out << ", aux_path=" << aux_path_;
  }
}

} // namespace stp
