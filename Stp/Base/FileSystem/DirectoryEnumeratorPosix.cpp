// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/DirectoryEnumerator.h"

#include <fnmatch.h>

namespace stp {

SystemErrorCode DirectoryEnumerator::tryOpen(const FilePath& path, StringSpan pattern) {
  ASSERT(!isOpen());
  pattern_ = pattern;
  return tryOpen(path);
}

SystemErrorCode DirectoryEnumerator::tryOpen(const FilePath& path) {
  ASSERT(!isOpen());
  DIR* dir = ::opendir(toNullTerminated(path));
  if (dir) {
    current_dir_ = dir;
    return PosixErrorCode::Ok;
  }
  return lastPosixErrorCode();
}

void DirectoryEnumerator::close() {
  ASSERT(isOpen());
  DIR* dir = exchange(current_dir_, nullptr);
  if (::closedir(dir) != 0)
    ASSERT(false);
}

static bool isDotEntry(const FilePathChar* basename) {
  if (basename[0] != '.')
    return false;
  if (basename[1] == '\0')
    return true;
  return basename[1] == '.' && basename[2] == '\0';
}

bool DirectoryEnumerator::tryMoveNext(SystemErrorCode& out_error_code) {
  ASSERT(isOpen());
  DIR* dir = current_dir_;

  errno = 0;
  struct dirent* dent;
  while ((dent = ::readdir(dir)) != nullptr) {
    if (isDotEntry(dent->d_name))
      continue;

    if (!pattern_.isEmpty()) {
      if (::fnmatch(toNullTerminated(pattern_), dent->d_name, FNM_NOESCAPE) != 0)
        continue;
    }
    dirent_ = dent;
    return true;
  }
  dirent_ = nullptr;
  out_error_code = lastPosixErrorCode();
  return false;
}

} // namespace stp
