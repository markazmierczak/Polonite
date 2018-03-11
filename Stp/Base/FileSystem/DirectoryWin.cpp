// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/Directory.h"

namespace stp {

bool Directory::exists(const FilePath& path) {
  DWORD fileattr = ::GetFileAttributesW(toNullTerminated(path));
  if (fileattr != INVALID_FILE_ATTRIBUTES)
    return (fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0;
  return false;
}

SystemErrorCode Directory::tryCreate(const FilePath& path) {
  if (::CreateDirectoryW(toNullTerminated(path), nullptr) != 0)
    return WinErrorCode::Success;

  auto error = lastWinErrorCode();
  if (error.GetCode() == WinErrorCode::AlreadyExists) {
    // This error code doesn't indicate whether we were racing with someone
    // creating the same directory, or a file with the same path.
    if (exists(path))
      return WinErrorCode::Success;
  }
  return error;
}

SystemErrorCode Directory::tryRemoveEmpty(const FilePath& path) {
  if (!::RemoveDirectoryW(toNullTerminated(path)))
    return lastWinErrorCode();
  return WinErrorCode::Success;
}

SystemErrorCode Directory::tryGetDriveSpaceInfo(const FilePath& path, DriveSpaceInfo& out_space) {
  ULARGE_INTEGER available, total, free;
  if (!::GetDiskFreeSpaceExW(toNullTerminated(path), &available, &total, &free))
    return lastWinErrorCode();

  auto ulargeIntToInt64 = [](ULARGE_INTEGER bytes) {
    ASSERT(bytes.QuadPart <= INT64_MAX);
    return static_cast<int64_t>(bytes.QuadPart);
  };
  out_space.available = ulargeIntToInt64(available);
  out_space.total = ulargeIntToInt64(total);
  out_space.free = ulargeIntToInt64(free);
  return WinErrorCode::Success;
}

} // namespace stp
