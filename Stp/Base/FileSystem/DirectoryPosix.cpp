// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/Directory.h"

#include "Base/Posix/EintrWrapper.h"
#include "Base/Posix/StatWrapper.h"

#include <unistd.h>

#if OS(ANDROID)
# include <sys/vfs.h>
# define statvfs statfs  // Android uses a statvfs-like statfs struct and call.
#else
# include <sys/statvfs.h>
#endif

#if OS(LINUX)
# include <linux/magic.h>
# include <sys/vfs.h>
#endif

namespace stp {

bool Directory::Exists(const FilePath& path) {
  stat_wrapper_t file_info;
  if (posix::CallStat(toNullTerminated(path), &file_info) == 0)
    return S_ISDIR(file_info.st_mode);
  return false;
}

SystemErrorCode Directory::TryCreate(const FilePath& path) {
  if (::mkdir(toNullTerminated(path), 0775) == 0)
    return PosixErrorCode::Ok;
  auto error_code = getLastSystemErrorCode();
  if (Exists(path))
    return PosixErrorCode::Ok;
  return error_code;
}

SystemErrorCode Directory::tryRemoveEmpty(const FilePath& path) {
  if (::rmdir(toNullTerminated(path)) == 0)
    return PosixErrorCode::Ok;
  return getLastPosixErrorCode();
}

#if OS(LINUX)
static bool IsStatsZeroIfUnlimited(const FilePath& path) {
  struct statfs stats;
  if (HANDLE_EINTR(::statfs(toNullTerminated(path), &stats)) != 0)
    return false;

  switch (stats.f_type) {
    case TMPFS_MAGIC:
    case HUGETLBFS_MAGIC:
    case RAMFS_MAGIC:
      return true;
  }
  return false;
}
#endif

SystemErrorCode Directory::tryGetDriveSpaceInfo(const FilePath& path, DriveSpaceInfo& out_space) {
  struct statvfs stats;
  if (HANDLE_EINTR(::statvfs(toNullTerminated(path), &stats)) != 0)
    return getLastPosixErrorCode();

  #if OS(LINUX)
  const bool zero_size_means_unlimited = stats.f_blocks == 0 && IsStatsZeroIfUnlimited(path);
  #else
  const bool zero_size_means_unlimited = false;
  #endif

  auto Normalize = [zero_size_means_unlimited](int64_t blocks, int64_t frsize) {
    return zero_size_means_unlimited ? Limits<int64_t>::Max : (blocks * frsize);
  };
  out_space.total = Normalize(stats.f_blocks, stats.f_frsize);
  out_space.free = Normalize(stats.f_bfree, stats.f_frsize);
  out_space.available = Normalize(stats.f_bavail, stats.f_frsize);
  return PosixErrorCode::Ok;
}

} // namespace stp
