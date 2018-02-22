// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/DirectoryEnumerator.h"

#include "Base/FileSystem/FileSystemException.h"
#include "Base/Text/Utf.h"

namespace stp {

DirectoryEnumerator::DirectoryEnumerator() {
}

DirectoryEnumerator::~DirectoryEnumerator() {
  if (isOpen())
    close();
}

void DirectoryEnumerator::open(const FilePath& path) {
  ASSERT(!isOpen());
  auto error_code = tryOpen(path);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
}

void DirectoryEnumerator::open(const FilePath& path, StringSpan pattern) {
  ASSERT(!isOpen());
  auto error_code = tryOpen(path, pattern);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
}

bool DirectoryEnumerator::moveNext() {
  SystemErrorCode error_code;
  bool has_next = tryMoveNext(error_code);
  if (!has_next && !isOk(error_code))
    throw FileSystemException(error_code);
  return has_next;
}

} // namespace stp
