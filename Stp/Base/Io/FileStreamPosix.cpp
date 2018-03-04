// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/FileStream.h"

#include "Base/FileSystem/FileSystemException.h"
#include "Base/Debug/Log.h"
#include "Base/Io/FileStreamInfo.h"
#include "Base/Io/IoException.h"
#include "Base/Posix/EintrWrapper.h"
#include "Base/Posix/PosixErrorCode.h"
#include "Base/Posix/StatWrapper.h"

#include <fcntl.h>
#include <unistd.h>

namespace stp {

static_assert(static_cast<int>(SeekOrigin::Begin) == SEEK_SET &&
              static_cast<int>(SeekOrigin::Current) == SEEK_CUR &&
              static_cast<int>(SeekOrigin::End) == SEEK_END,
              "SeekOrigin must match the system headers");

SystemErrorCode FileStream::tryOpenInternal(
    const FilePath& path, FileMode mode, FileAccess access) {
  ASSERT(!isOpen());

  int mode_flags = 0;
  switch (mode) {
    case FileMode::Create:
      mode_flags = O_CREAT | O_TRUNC;
      break;
    case FileMode::CreateNew:
      mode_flags = O_CREAT | O_EXCL;
      break;
    case FileMode::OpenExisting:
      mode_flags = 0;
      break;
    case FileMode::OpenTruncated:
      mode_flags = O_TRUNC;
      break;
    case FileMode::OpenOrCreate:
      mode_flags = O_CREAT;
      break;
    case FileMode::Append:
      ASSERT(access == FileAccess::WriteOnly);
      mode_flags = O_APPEND;
      break;
  }

  int access_flags = 0;
  switch (access) {
    case FileAccess::ReadOnly:
      access_flags = O_RDONLY;
      break;
    case FileAccess::WriteOnly:
      access_flags = O_WRONLY;
      break;
    case FileAccess::ReadWrite:
      access_flags = O_RDWR;
      break;
  }

  int perm_flags = 0664;
  int open_flags = mode_flags | access_flags;
  int descriptor = HANDLE_EINTR(::open(toNullTerminated(path), open_flags, perm_flags));
  if (descriptor == -1)
    return getLastPosixErrorCode();

  native_.reset(descriptor);
  access_ = access;
  #if ASSERT_IS_ON
  append_ = mode == FileMode::Append;
  #endif
  return SystemErrorCode::Ok;
}

void FileStream::closeInternal(NativeFile fd) {
  int rv = IGNORE_EINTR(::close(fd));
  if (rv != 0)
    throw SystemException(getLastPosixErrorCode());
}

int FileStream::readAtMost(MutableBufferSpan output) {
  ASSERT(canRead());
  int bytes_read = 0;
  int fd = native_.get();
  do {
    ssize_t prv = ::read(fd, output.data(), toUnsigned(output.size()));
    ASSERT(-1 <= prv && prv <= output.size());
    int rv = static_cast<int>(prv);
    if (rv > 0) {
      output.removePrefix(rv);
      bytes_read += rv;
    } else {
      if (rv == 0)
        break;
      if (errno != EINTR)
        throw SystemException(getLastPosixErrorCode());
    }
  } while (!output.isEmpty());
  return bytes_read;
}

void FileStream::write(BufferSpan input) {
  ASSERT(canWrite());
  int fd = native_.get();
  do {
    ssize_t prv = ::write(fd, input.data(), toUnsigned(input.size()));
    ASSERT(-1 <= prv && prv <= input.size());
    int rv = static_cast<int>(prv);
    if (rv >= 0) {
      // Conformant POSIX implementations must not return zero on error.
      ASSERT(rv != 0 || input.isEmpty());
      input.removePrefix(rv);
    } else {
      if (errno != EINTR)
        throw SystemException(getLastPosixErrorCode());
    }
  } while (!input.isEmpty());
}

#if OS(ANDROID)
// Bionic uses 32-bit off_t by default.
// Delegate to 64-bit variants.
// This is not an issue on Linux because __USE_FILE_OFFSET64 is set.
#define pread pread64
#define pwrite pwrite64
#define lseek lseek64
#define ftruncate ftruncate64
#endif

void FileStream::positionalRead(int64_t offset, MutableBufferSpan output) {
  ASSERT(canRead() && canSeek());
  ASSERT(offset >= 0);
  int fd = native_.get();
  do {
    ssize_t prv = ::pread(fd, output.data(), toUnsigned(output.size()), offset);
    ASSERT(-1 <= prv && prv <= output.size());
    int rv = static_cast<int>(prv);
    if (rv > 0) {
      offset += rv;
      output.removePrefix(rv);
    } else {
      if (rv == 0) {
        if (!output.isEmpty()) // zero length was requested ?
          throw EndOfStreamException();
      } else if (errno != EINTR) {
        throw SystemException(getLastPosixErrorCode());
      }
    }
  } while (!output.isEmpty());
}

void FileStream::positionalWrite(int64_t offset, BufferSpan input) {
  ASSERT(canWrite() && canSeek());
  // Linux has bug and does not follow POSIX requirements.
  // Thus we disable positional write on all systems in this mode.
  ASSERT(!append_);
  int fd = native_.get();
  do {
    ssize_t prv = ::pwrite(fd, input.data(), toUnsigned(input.size()), offset);
    ASSERT(-1 <= prv && prv <= input.size());
    int rv = static_cast<int>(prv);
    if (rv >= 0) {
      // Conformant POSIX implementations must not return zero on error.
      ASSERT(rv != 0 || input.isEmpty());
      offset += rv;
      input.removePrefix(rv);
    } else {
      if (errno != EINTR)
        throw SystemException(getLastPosixErrorCode());
    }
  } while (!input.isEmpty());
}

int64_t FileStream::seek(int64_t offset, SeekOrigin origin) {
  ASSERT(canSeek());
  int64_t rv = ::lseek(native_.get(), offset, static_cast<int>(origin));
  ASSERT(rv >= -1);
  if (rv < 0)
    throw SystemException(getLastPosixErrorCode());
  return rv;
}

bool FileStream::canSeekInternal() {
  ASSERT(isOpen());

  stat_wrapper_t file_info;
  if (posix::callFstat(native_.get(), &file_info) != 0)
    return false;

  switch (file_info.st_mode & S_IFMT) {
    case S_IFDIR:
    case S_IFREG:
    case S_IFBLK:
      return true;
  }
  return false;
}

int64_t FileStream::getLength() {
  ASSERT(isOpen());

  stat_wrapper_t file_info;
  if (posix::callFstat(native_.get(), &file_info) != 0)
    throw SystemException(getLastPosixErrorCode());

  return file_info.st_size;
}

void FileStream::setLength(int64_t length) {
  ASSERT(length >= 0);
  ASSERT(isOpen());

  int rv = HANDLE_EINTR(::ftruncate(native_.get(), length));
  if (rv != 0)
    throw SystemException(getLastPosixErrorCode());
}

void FileStream::syncToDisk() {
  ASSERT(isOpen());

  #if OS(LINUX) || OS(ANDROID)
  int rv = HANDLE_EINTR(::fdatasync(native_.get()));
  #else
  int rv = HANDLE_EINTR(::fsync(native_.get()));
  #endif
  if (rv != 0)
    throw SystemException(getLastPosixErrorCode());
}

void FileStream::getInfo(FileStreamInfo& out) {
  ASSERT(isOpen());
  if (posix::callFstat(native_.get(), &out.stat_) != 0)
    throw SystemException(getLastPosixErrorCode());
}

void FileStream::setTimes(Time last_accessed, Time last_modified, Time creation_time) {
  ASSERT(creation_time.isNull(), "creation_time not supported on POSIX");
  ASSERT(!last_accessed.isNull());
  ASSERT(!last_modified.isNull());

  ASSERT(isOpen());

  #ifdef __USE_XOPEN2K8
  // futimens should be available, but futimes might not be
  // http://pubs.opengroup.org/onlinepubs/9699919799/
  timespec times[2];
  times[0] = last_accessed.ToTimeSpec();
  times[1] = last_modified.ToTimeSpec();
  int rv = ::futimens(native_.get(), times);
  #else
  timeval times[2];
  times[0] = last_accessed.ToTimeVal();
  times[1] = last_modified.ToTimeVal();
  int rv = ::futimes(native_.get(), times);
  #endif

  if (rv != 0)
    throw SystemException(getLastPosixErrorCode());
}

} // namespace stp
