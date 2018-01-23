// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Fs/DirectoryEnumerator.h"

#include "Base/Posix/PosixErrorCode.h"

#include <fnmatch.h>

namespace stp {

ErrorCode DirectoryEnumerator::TryOpen(const FilePath& path, StringSpan pattern) {
  ASSERT(!IsOpen());
  pattern_ = pattern;
  return TryOpen(path);
}

ErrorCode DirectoryEnumerator::TryOpen(const FilePath& path) {
  ASSERT(!IsOpen());
  DIR* dir = ::opendir(ToNullTerminated(path));
  if (dir) {
    current_dir_ = dir;
    return ErrorCode();
  }
  return GetLastPosixErrorCode();
}

void DirectoryEnumerator::Close() noexcept {
  ASSERT(IsOpen());
  DIR* dir = Exchange(current_dir_, nullptr);
  if (::closedir(dir) != 0)
    ASSERT(false);
}

static bool IsDotEntry(const FilePathChar* basename) {
  if (basename[0] != '.')
    return false;
  if (basename[1] == '\0')
    return true;
  return basename[1] == '.' && basename[2] == '\0';
}

bool DirectoryEnumerator::TryMoveNext(ErrorCode& out_error_code) {
  ASSERT(IsOpen());
  DIR* dir = current_dir_;

  errno = 0;
  struct dirent* dent;
  while ((dent = ::readdir(dir)) != nullptr) {
    if (IsDotEntry(dent->d_name))
      continue;

    if (!pattern_.IsEmpty()) {
      if (::fnmatch(ToNullTerminated(pattern_), dent->d_name, FNM_NOESCAPE) != 0)
        continue;
    }
    dirent_ = dent;
    return true;
  }
  dirent_ = nullptr;
  out_error_code = GetLastPosixErrorCode();
  return false;
}

} // namespace stp
