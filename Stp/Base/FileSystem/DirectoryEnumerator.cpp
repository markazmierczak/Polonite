// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/DirectoryEnumerator.h"

#include "Base/FileSystem/FileSystemException.h"
#include "Base/Text/Utf.h"

namespace stp {

DirectoryEnumerator::DirectoryEnumerator() {
}

DirectoryEnumerator::~DirectoryEnumerator() {
  if (IsOpen())
    Close();
}

void DirectoryEnumerator::Open(const FilePath& path) {
  ASSERT(!IsOpen());
  auto error_code = TryOpen(path);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
}

void DirectoryEnumerator::Open(const FilePath& path, StringSpan pattern) {
  ASSERT(!IsOpen());
  auto error_code = TryOpen(path, pattern);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
}

bool DirectoryEnumerator::MoveNext() {
  SystemErrorCode error_code;
  bool has_next = TryMoveNext(error_code);
  if (!has_next && !IsOk(error_code))
    throw FileSystemException(error_code);
  return has_next;
}

} // namespace stp
