// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/Directory.h"

namespace stp {

bool Directory::Exists(const FilePath& path) {
  DWORD fileattr = ::GetFileAttributesW(ToNullTerminated(path));
  if (fileattr != INVALID_FILE_ATTRIBUTES)
    return (fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0;
  return false;
}

SystemErrorCode Directory::TryCreate(const FilePath& path) {
  if (::CreateDirectoryW(ToNullTerminated(path), nullptr) != 0)
    return WinErrorCode::Success;

  auto error = GetLastWinErrorCode();
  if (error.GetCode() == WinErrorCode::AlreadyExists) {
    // This error code doesn't indicate whether we were racing with someone
    // creating the same directory, or a file with the same path.
    if (Exists(path))
      return WinErrorCode::Success;
  }
  return error;
}

SystemErrorCode Directory::tryRemoveEmpty(const FilePath& path) {
  if (!::RemoveDirectoryW(ToNullTerminated(path)))
    return GetLastWinErrorCode();
  return WinErrorCode::Success;
}

SystemErrorCode Directory::tryGetDriveSpaceInfo(const FilePath& path, DriveSpaceInfo& out_space) {
  ULARGE_INTEGER available, total, free;
  if (!::GetDiskFreeSpaceExW(ToNullTerminated(path), &available, &total, &free))
    return GetLastWinErrorCode();

  auto ULargeIntToInt64 = [](ULARGE_INTEGER bytes) {
    ASSERT(bytes.QuadPart <= INT64_MAX);
    return static_cast<int64_t>(bytes.QuadPart);
  };
  out_space.available = ULargeIntToInt64(available);
  out_space.total = ULargeIntToInt64(total);
  out_space.free = ULargeIntToInt64(free);
  return WinErrorCode::Success;
}

} // namespace stp
