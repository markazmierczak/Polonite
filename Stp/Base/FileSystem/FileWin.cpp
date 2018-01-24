// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/File.h"

#include "Base/FileSystem/FileInfo.h"
#include "Base/Io/FileStream.h"
#include "Base/Text/Format.h"
#include "Base/Util/Finally.h"
#include "Base/Win/WinErrorCode.h"

#include <io.h>

namespace stp {

bool File::Exists(const FilePath& path) {
  return ::GetFileAttributesW(ToNullTerminated(path)) != INVALID_FILE_ATTRIBUTES;
}

ErrorCode File::TryGetInfo(const FilePath& path, FileInfo& out) {
  if (!::GetFileAttributesExW(ToNullTerminated(path), GetFileExInfoStandard, &out.attr_data_))
    return GetLastWinErrorCode();
  return ErrorCode();
}

ErrorCode File::TryMakeAbsolutePath(const FilePath& input, FilePath& output) {
  output.Clear();

  int buffer_length = 1;
  while (true) {
    wchar_t* dst = output.chars().AppendUninitialized(buffer_length - 1);
    int rv = static_cast<int>(
        ::GetFullPathNameW(ToNullTerminated(input), buffer_length, dst, nullptr));
    if (rv < buffer_length) {
      if (rv == 0)
        return GetLastWinErrorCode();
      output.Truncate(rv);
      return ErrorCode();
    }
    buffer_length = rv;
    output.Clear();
  }
}

ErrorCode File::TryMakeLongPath(const FilePath& input, FilePath& output) {
  output.Clear();

  int buffer_length = 1;
  while (true) {
    wchar_t* dst = output.chars().AppendUninitialized(buffer_length - 1);
    int rv = static_cast<int>(
        ::GetLongPathNameW(ToNullTerminated(input), buffer_length, dst, nullptr));
    if (rv < buffer_length) {
      if (rv == 0)
        return GetLastWinErrorCode();
      output.Truncate(rv);
      return ErrorCode();
    }
    buffer_length = rv;
    output.Clear();
  }
}

ErrorCode File::TryDelete(const FilePath& path) {
  if (::DeleteFileW(ToNullTerminated(path)) != 0)
    return ErrorCode();

  auto error = GetLastWinErrorCode();

  if (error.GetCode() == WinErrorCode::AccessDenied) {
    DWORD attr = ::GetFileAttributesW(ToNullTerminated(path));
    if (attr == INVALID_FILE_ATTRIBUTES)
      return error;

    // We may need to clear the read-only bit.
    if ((attr & FILE_ATTRIBUTE_READONLY) &&
        !::SetFileAttributesW(ToNullTerminated(path), attr & ~FILE_ATTRIBUTE_READONLY)) {
      return GetLastWinErrorCode();
    }
    // Second chance.
    if (::DeleteFileW(ToNullTerminated(path)) != 0)
      return ErrorCode();

    error = GetLastWinErrorCode();
  }
  return error;
}

ErrorCode File::TryDeleteAfterReboot(const FilePath& path) {
  DWORD flags = MOVEFILE_DELAY_UNTIL_REBOOT | MOVEFILE_REPLACE_EXISTING;
  if (!::MoveFileExW(ToNullTerminated(path), NULL, flags))
    return GetLastWinErrorCode();
  return ErrorCode();
}

ErrorCode File::TryReplace(const FilePath& from, const FilePath& to) {
  // Try a simple move first. It will only succeed when |to| doesn't already exist.
  if (::MoveFileW(ToNullTerminated(from), ToNullTerminated(to)))
    return ErrorCode();

  // Try the full-blown replace if the move fails, as ReplaceFile will only
  // succeed when |to| does exist. When writing to a network share, we may
  // not be able to change the ACLs. Ignore ACL errors then (REPLACEFILE_IGNORE_MERGE_ERRORS).
  if (::ReplaceFileW(ToNullTerminated(to), ToNullTerminated(from), NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL))
    return ErrorCode();

  return GetLastWinErrorCode();
}

ErrorCode File::TryCreateTemporaryIn(const FilePath& dir, FilePath& output_path) {
  wchar_t temp_name[MAX_PATH + 1];

  if (!::GetTempFileNameW(ToNullTerminated(dir), L"", 0, temp_name))
    return GetLastWinErrorCode();

  auto temp_cpath = FilePath::FromNullTerminated(temp_name);
  ErrorCode long_rv = TryMakeLongPath(temp_cpath, output_path);
  if (!IsOk(long_rv)) {
    // GetLongPathNameW() failed, but we still have a temporary file.
    output_path = temp_cpath;
  }
  return ErrorCode();
}

} // namespace stp
