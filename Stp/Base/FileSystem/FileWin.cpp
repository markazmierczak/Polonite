// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/File.h"

#include "Base/FileSystem/FileInfo.h"
#include "Base/Io/FileStream.h"
#include "Base/Text/FormatMany.h"
#include "Base/Util/Finally.h"

#include <io.h>

namespace stp {

bool File::Exists(const FilePath& path) {
  return ::GetFileAttributesW(toNullTerminated(path)) != INVALID_FILE_ATTRIBUTES;
}

SystemErrorCode File::tryGetInfo(const FilePath& path, FileInfo& out) {
  if (!::GetFileAttributesExW(toNullTerminated(path), GetFileExInfoStandard, &out.attr_data_))
    return GetLastWinErrorCode();
  return WinErrorCode::Success;
}

SystemErrorCode File::TryMakeAbsolutePath(const FilePath& input, FilePath& output) {
  output.clear();

  int buffer_length = 1;
  while (true) {
    wchar_t* dst = output.chars().appendUninitialized(buffer_length - 1);
    int rv = static_cast<int>(
        ::GetFullPathNameW(toNullTerminated(input), buffer_length, dst, nullptr));
    if (rv < buffer_length) {
      if (rv == 0)
        return GetLastWinErrorCode();
      output.truncate(rv);
      return WinErrorCode::Success;
    }
    buffer_length = rv;
    output.clear();
  }
}

SystemErrorCode File::TryMakeLongPath(const FilePath& input, FilePath& output) {
  output.clear();

  int buffer_length = 1;
  while (true) {
    wchar_t* dst = output.chars().appendUninitialized(buffer_length - 1);
    int rv = static_cast<int>(
        ::GetLongPathNameW(toNullTerminated(input), buffer_length, dst, nullptr));
    if (rv < buffer_length) {
      if (rv == 0)
        return GetLastWinErrorCode();
      output.truncate(rv);
      return WinErrorCode::Success;
    }
    buffer_length = rv;
    output.clear();
  }
}

SystemErrorCode File::TryDelete(const FilePath& path) {
  if (!::DeleteFileW(toNullTerminated(path)))
    return GetLastWinErrorCode();
  return WinErrorCode::Success;
}

SystemErrorCode File::TryDeleteAfterReboot(const FilePath& path) {
  DWORD flags = MOVEFILE_DELAY_UNTIL_REBOOT | MOVEFILE_REPLACE_EXISTING;
  if (!::MoveFileExW(toNullTerminated(path), NULL, flags))
    return GetLastWinErrorCode();
  return WinErrorCode::Success;
}

SystemErrorCode File::TryReplace(const FilePath& from, const FilePath& to) {
  // Try a simple move first. It will only succeed when |to| doesn't already exist.
  if (::MoveFileW(toNullTerminated(from), toNullTerminated(to)))
    return WinErrorCode::Success;

  // Try the full-blown replace if the move fails, as ReplaceFile will only
  // succeed when |to| does exist. When writing to a network share, we may
  // not be able to change the ACLs. Ignore ACL errors then (REPLACEFILE_IGNORE_MERGE_ERRORS).
  if (::ReplaceFileW(toNullTerminated(to), toNullTerminated(from), NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL))
    return WinErrorCode::Success;

  return GetLastWinErrorCode();
}

SystemErrorCode File::TryCreateTemporaryIn(const FilePath& dir, FilePath& output_path) {
  wchar_t temp_name[MAX_PATH + 1];

  if (!::GetTempFileNameW(toNullTerminated(dir), L"", 0, temp_name))
    return GetLastWinErrorCode();

  auto temp_cpath = FilePath::FromNullTerminated(temp_name);
  WinErrorCode long_rv = TryMakeLongPath(temp_cpath, output_path);
  if (!IsOk(long_rv)) {
    // GetLongPathNameW() failed, but we still have a temporary file.
    output_path = temp_cpath;
  }
  return WinErrorCode::Success;
}

} // namespace stp
