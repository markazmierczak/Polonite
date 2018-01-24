// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/DirectoryEnumerator.h"

#include "Base/Text/StringUtfConversions.h"
#include "Base/Time/Time.h"
#include "Base/Win/WinErrorCode.h"

namespace stp {

ErrorCode DirectoryEnumerator::TryOpen(const FilePath& path, StringSpan pattern) {
  AppendUnicode(pattern_, pattern);
  return TryOpen(path);
}

ErrorCode DirectoryEnumerator::TryOpen(const FilePath& path) {
  ASSERT(!IsOpen());
  search_path_.Clear();
  search_path_.EnsureCapacity(path.size() + pattern_.size() + 2);
  search_path_ = path;

  auto search_ops =  FindExSearchNameMatch;
  if (pattern_.IsEmpty()) {
    search_path_.AddComponent(FILE_PATH_LITERAL("*"));
  } else {
    search_path_.AddComponent(pattern_);
  }

  find_handle_ = ::FindFirstFileExW(
      ToNullTerminated(search_path_),
      FindExInfoBasic, // Omit short name.
      &find_data_,
      search_ops,
      NULL,
      FIND_FIRST_EX_LARGE_FETCH);

  if (find_handle_ != INVALID_HANDLE_VALUE) {
    status_ = Status::AtFirst;
    return ErrorCode();
  }
  auto error = GetLastWinErrorCode();
  // An empty root directory has no entries (even dot entries).
  if (error == WinErrorCode::FileNotFound ||
      error == WinErrorCode::NoMoreFiles) {
    status_ = Status::Empty;
    return ErrorCode();
  }
  return error;
}

void DirectoryEnumerator::Close() noexcept {
  ASSERT(IsOpen());
  pattern_.Clear();
  if (status_ != Status::Empty) {
    HANDLE handle = Exchange(find_handle_, INVALID_HANDLE_VALUE);
    if (::FindClose(handle) == 0)
      ASSERT(false);
  }
  status_ = Status::Closed;
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
  if (status_ != Status::AtNext) {
    if (status_ == Status::Empty)
      return false;
    status_ = Status::AtNext;
    if (!IsDotEntry(find_data_.cFileName))
      return true;
  }
  while (::FindNextFileW(find_handle_, &find_data_) != 0) {
    if (IsDotEntry(find_data_.cFileName))
      continue;
    return true;
  }
  auto error = GetLastWinErrorCode();
  if (error == WinErrorCode::NoMoreFiles)
    out_error_code = ErrorCode();
  else
    out_error_code = error;
  return false;
}

uint64_t DirectoryEnumerator::GetSize() const {
  ULARGE_INTEGER size;
  size.HighPart = find_data_.nFileSizeHigh;
  size.LowPart  = find_data_.nFileSizeLow;
  return size.QuadPart;
}

Time DirectoryEnumerator::GetLastAccessTime() const {
  return Time::FromFileTime(find_data_.ftLastAccessTime);
}

Time DirectoryEnumerator::GetLastModifiedTime() const {
  return Time::FromFileTime(find_data_.ftLastWriteTime);
}

Time DirectoryEnumerator::GetCreationTime() const {
  return Time::FromFileTime(find_data_.ftCreationTime);
}

} // namespace stp
