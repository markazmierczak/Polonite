// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/File.h"

#include "Base/FileSystem/FileInfo.h"
#include "Base/FileSystem/FilePathWriter.h"
#include "Base/FileSystem/FileSystemException.h"
#include "Base/Io/FileStream.h"
#include "Base/App/Application.h"
#include "Base/Posix/EintrWrapper.h"
#include "Base/Posix/StatWrapper.h"
#include "Base/Type/Formattable.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

namespace stp {

bool File::exists(const FilePath& path) {
  return ::access(toNullTerminated(path), F_OK) == 0;
}

SystemErrorCode File::tryGetInfo(const FilePath& path, FileInfo& out) {
  if (posix::callStat(toNullTerminated(path), &out.stat_) != 0)
    return lastPosixErrorCode();
  return PosixErrorCode::Ok;
}

SystemErrorCode File::tryMakeAbsolutePath(const FilePath& input, FilePath& output) {
  char full_path[PATH_MAX + 1];
  if (::realpath(toNullTerminated(input), full_path) == nullptr)
    return lastPosixErrorCode();

  output = makeFilePathSpanFromNullTerminated(full_path);
  return PosixErrorCode::Ok;
}

SystemErrorCode File::tryRemove(const FilePath& path) {
  if (::unlink(toNullTerminated(path)) != 0)
    return lastPosixErrorCode();
  return PosixErrorCode::Ok;
}

SystemErrorCode File::tryReplace(const FilePath& from, const FilePath& to) {
  if (::rename(toNullTerminated(from), toNullTerminated(to)) != 0)
    return lastPosixErrorCode();
  return PosixErrorCode::Ok;
}

SystemErrorCode File::tryCreateTemporaryIn(const FilePath& dir, FilePath& output_path) {
  output_path.clear();
  output_path = dir;

  FilePathWriter writer(output_path);
  writer.ensureSeparator();
  writer << ".stp." << Application::instance().getName() << ".XXXXXX";

  // this should be OK since mkstemp just replaces characters in place
  char* buffer = const_cast<char*>(toNullTerminated(output_path));
  int rv = HANDLE_EINTR(::mkstemp(buffer));
  if (rv == -1)
    return lastPosixErrorCode();

  return PosixErrorCode::Ok;
}

SystemErrorCode File::tryCreateSymbolicLink(const FilePath& symlink, const FilePath& target) {
  if (::symlink(toNullTerminated(target), toNullTerminated(symlink)) != 0)
    return lastPosixErrorCode();
  return PosixErrorCode::Ok;
}

SystemErrorCode File::tryReadSymbolicLink(const FilePath& symlink, FilePath& out_target) {
  char buf[PATH_MAX];
  int count = ::readlink(toNullTerminated(symlink), buf, sizeof(buf));
  ASSERT(-1 <= count && count <= PATH_MAX);
  if (count < 0)
    return lastPosixErrorCode();

  out_target = FilePathSpan(buf, count);
  return PosixErrorCode::Ok;
}

void File::createSymbolicLink(const FilePath& symlink, const FilePath& target) {
  auto error_code = tryCreateSymbolicLink(symlink, target);
  if (!isOk(error_code))
    throw FileSystemException(error_code, symlink);
}

FilePath File::readSymbolicLink(const FilePath& symlink) {
  FilePath target;
  auto error_code = tryReadSymbolicLink(symlink, target);
  if (!isOk(error_code))
    throw FileSystemException(error_code, symlink);
  return target;
}

SystemErrorCode File::tryGetPosixPermissions(const FilePath& path, int& out_mode) {
  stat_wrapper_t file_info;
  if (posix::callStat(toNullTerminated(path), &file_info) != 0)
    return lastPosixErrorCode();
  out_mode = file_info.st_mode & 0777;
  return PosixErrorCode::Ok;
}

SystemErrorCode File::trySetPosixPermissions(const FilePath& path, int mode) {
  ASSERT((mode & ~0777) == 0);

  // Calls stat() so that we can preserve the higher bits like S_ISGID.
  stat_wrapper_t stat_buf;
  if (posix::callStat(toNullTerminated(path), &stat_buf) != 0)
    return lastPosixErrorCode();

  // Clears the existing permission bits, and adds the new ones.
  mode_t updated_mode_bits = stat_buf.st_mode & ~0777;
  updated_mode_bits |= mode;

  if (HANDLE_EINTR(::chmod(toNullTerminated(path), updated_mode_bits)) != 0)
    return lastPosixErrorCode();

  return PosixErrorCode::Ok;
}

int File::getPosixPermissions(const FilePath& path) {
  int mode;
  auto error_code = tryGetPosixPermissions(path, mode);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
  return mode;
}

void File::setPosixPermissions(const FilePath& path, int mode) {
  auto error_code = trySetPosixPermissions(path, mode);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
}

} // namespace stp
