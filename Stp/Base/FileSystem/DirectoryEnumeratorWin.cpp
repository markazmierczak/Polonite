// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/DirectoryEnumerator.h"

#include "Base/Text/StringUtfConversions.h"
#include "Base/Time/Time.h"

namespace stp {

SystemErrorCode DirectoryEnumerator::tryOpen(const FilePath& path, StringSpan pattern) {
  appendUnicode(pattern_, pattern);
  return tryOpen(path);
}

SystemErrorCode DirectoryEnumerator::tryOpen(const FilePath& path) {
  ASSERT(!isOpen());
  search_path_.clear();
  search_path_.ensureCapacity(path.size() + pattern_.size() + 2);
  search_path_ = path;

  auto search_ops =  FindExSearchNameMatch;
  if (pattern_.isEmpty()) {
    search_path_.addComponent(FILE_PATH_LITERAL("*"));
  } else {
    search_path_.addComponent(pattern_);
  }

  find_handle_ = ::FindFirstFileExW(
      toNullTerminated(search_path_),
      FindExInfoBasic, // Omit short name.
      &find_data_,
      search_ops,
      NULL,
      FIND_FIRST_EX_LARGE_FETCH);

  if (find_handle_ != INVALID_HANDLE_VALUE) {
    status_ = Status::AtFirst;
    return WinErrorCode::Success;
  }
  auto error = getLastWinErrorCode();
  // An empty root directory has no entries (even dot entries).
  if (error == WinErrorCode::FileNotFound ||
      error == WinErrorCode::NoMoreFiles) {
    status_ = Status::Empty;
    return WinErrorCode::Success;
  }
  return error;
}

void DirectoryEnumerator::close() {
  ASSERT(isOpen());
  pattern_.clear();
  if (status_ != Status::Empty) {
    HANDLE handle = exchange(find_handle_, INVALID_HANDLE_VALUE);
    if (::FindClose(handle) == 0)
      ASSERT(false);
  }
  status_ = Status::Closed;
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
  if (status_ != Status::AtNext) {
    if (status_ == Status::Empty)
      return false;
    status_ = Status::AtNext;
    if (!isDotEntry(find_data_.cFileName))
      return true;
  }
  while (::FindNextFileW(find_handle_, &find_data_) != 0) {
    if (isDotEntry(find_data_.cFileName))
      continue;
    return true;
  }
  auto error = getLastWinErrorCode();
  if (error == WinErrorCode::NoMoreFiles)
    out_error_code = WinErrorCode::Sucess;
  else
    out_error_code = error;
  return false;
}

uint64_t DirectoryEnumerator::getSize() const {
  ULARGE_INTEGER size;
  size.HighPart = find_data_.nFileSizeHigh;
  size.LowPart  = find_data_.nFileSizeLow;
  return size.QuadPart;
}

Time DirectoryEnumerator::lastAccessTime() const {
  return Time::fromFileTime(find_data_.ftLastAccessTime);
}

Time DirectoryEnumerator::lastModifiedTime() const {
  return Time::fromFileTime(find_data_.ftLastWriteTime);
}

Time DirectoryEnumerator::getCreationTime() const {
  return Time::fromFileTime(find_data_.ftCreationTime);
}

} // namespace stp
