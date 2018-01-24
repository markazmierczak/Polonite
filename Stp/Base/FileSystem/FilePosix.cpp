// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/File.h"

#include "Base/FileSystem/FileInfo.h"
#include "Base/FileSystem/FilePathWriter.h"
#include "Base/FileSystem/FileSystemException.h"
#include "Base/Io/FileStream.h"
#include "Base/App/Application.h"
#include "Base/Posix/EintrWrapper.h"
#include "Base/Posix/PosixErrorCode.h"
#include "Base/Posix/StatWrapper.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

namespace stp {

bool File::Exists(const FilePath& path) {
  return ::access(ToNullTerminated(path), F_OK) == 0;
}

ErrorCode File::TryGetInfo(const FilePath& path, FileInfo& out) {
  if (posix::CallStat(ToNullTerminated(path), &out.stat_) != 0)
    return GetLastPosixErrorCode();
  return ErrorCode();
}

ErrorCode File::TryMakeAbsolutePath(const FilePath& input, FilePath& output) {
  char full_path[PATH_MAX + 1];
  if (::realpath(ToNullTerminated(input), full_path) == nullptr)
    return GetLastPosixErrorCode();

  output = MakeFilePathSpanFromNullTerminated(full_path);
  return ErrorCode();
}

ErrorCode File::TryDelete(const FilePath& path) {
  if (::unlink(ToNullTerminated(path)) != 0)
    return GetLastPosixErrorCode();
  return ErrorCode();
}

ErrorCode File::TryReplace(const FilePath& from, const FilePath& to) {
  if (::rename(ToNullTerminated(from), ToNullTerminated(to)) != 0)
    return GetLastPosixErrorCode();
  return ErrorCode();
}

ErrorCode File::TryCreateTemporaryIn(const FilePath& dir, FilePath& output_path) {
  output_path.Clear();
  output_path = dir;

  FilePathWriter writer(output_path);
  writer.EnsureSeparator();
  writer.WriteAscii(".stp.");
  writer.WriteAscii(Application::Instance().GetName());
  writer.WriteAscii(".XXXXXX");

  // this should be OK since mkstemp just replaces characters in place
  char* buffer = const_cast<char*>(ToNullTerminated(output_path));
  int rv = HANDLE_EINTR(::mkstemp(buffer));
  if (rv == -1)
    return GetLastPosixErrorCode();

  return ErrorCode();
}

ErrorCode File::TryCreateSymbolicLink(const FilePath& symlink, const FilePath& target) {
  if (::symlink(ToNullTerminated(target), ToNullTerminated(symlink)) != 0)
    return GetLastPosixErrorCode();
  return ErrorCode();
}

ErrorCode File::TryReadSymbolicLink(const FilePath& symlink, FilePath& out_target) {
  char buf[PATH_MAX];
  int count = ::readlink(ToNullTerminated(symlink), buf, sizeof(buf));
  ASSERT(-1 <= count && count <= PATH_MAX);
  if (count < 0)
    return GetLastPosixErrorCode();

  out_target = FilePathSpan(buf, count);
  return ErrorCode();
}

void File::CreateSymbolicLink(const FilePath& symlink, const FilePath& target) {
  auto error_code = TryCreateSymbolicLink(symlink, target);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, symlink);
}

FilePath File::ReadSymbolicLink(const FilePath& symlink) {
  FilePath target;
  auto error_code = TryReadSymbolicLink(symlink, target);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, symlink);
  return target;
}

ErrorCode File::TryGetPosixPermissions(const FilePath& path, int& out_mode) {
  stat_wrapper_t file_info;
  if (posix::CallStat(ToNullTerminated(path), &file_info) != 0)
    return GetLastPosixErrorCode();
  out_mode = file_info.st_mode & 0777;
  return ErrorCode();
}

ErrorCode File::TrySetPosixPermissions(const FilePath& path, int mode) {
  ASSERT((mode & ~0777) == 0);

  // Calls stat() so that we can preserve the higher bits like S_ISGID.
  stat_wrapper_t stat_buf;
  if (posix::CallStat(ToNullTerminated(path), &stat_buf) != 0)
    return GetLastPosixErrorCode();

  // Clears the existing permission bits, and adds the new ones.
  mode_t updated_mode_bits = stat_buf.st_mode & ~0777;
  updated_mode_bits |= mode;

  if (HANDLE_EINTR(::chmod(ToNullTerminated(path), updated_mode_bits)) != 0)
    return GetLastPosixErrorCode();

  return ErrorCode();
}

int File::GetPosixPermissions(const FilePath& path) {
  int mode;
  auto error_code = TryGetPosixPermissions(path, mode);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
  return mode;
}

void File::SetPosixPermissions(const FilePath& path, int mode) {
  auto error_code = TrySetPosixPermissions(path, mode);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
}

} // namespace stp
