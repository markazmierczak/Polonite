// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Posix/FileDescriptor.h"

#include "Base/Error/SystemException.h"

#include <fcntl.h>

namespace stp {
namespace posix {

FileDescriptor& FileDescriptor::operator=(FileDescriptor&& other) noexcept {
  ASSERT(this != &other);
  FileDescriptor tmp(Exchange(fd_, Exchange(other.fd_, InvalidFd)));
  return *this;
}

void FileDescriptor::Close() {
  ASSERT(IsValid());
  // It's important to crash here.
  // There are security implications to not closing a file descriptor properly.
  // As file descriptors are "capabilities", keeping them open would make the
  // current process keep access to a resource.
  int ret = IGNORE_EINTR(::close(fd_));
  if (ret != 0) {
    throw Exception::WithDebug(
        SystemException(GetLastPosixErrorCode()), "closing descriptor failed");
  }
  fd_ = InvalidFd;
}

FileDescriptor FileDescriptor::Duplicate() {
  FileDescriptor rv = TryDuplicate();
  if (!rv.IsValid()) {
    throw Exception::WithDebug(
        SystemException(GetLastPosixErrorCode()), "failed to duplicate file descriptor");
  }
  return rv;
}

FileDescriptor FileDescriptor::DuplicateTo(int new_fd) {
  FileDescriptor rv = TryDuplicateTo(new_fd);
  if (!rv.IsValid()) {
    throw Exception::WithDebug(
        SystemException(GetLastPosixErrorCode()), "failed to duplicate file descriptor");
  }
  return rv;
}

void FileDescriptor::SetNonBlocking() {
  int flags = ::fcntl(fd_, F_GETFL);

  if (flags == -1) {
    throw Exception::WithDebug(
        SystemException(GetLastPosixErrorCode()), "unable to get descriptor flags");
  }

  if (!(flags & O_NONBLOCK)) {
    if (HANDLE_EINTR(::fcntl(fd_, F_SETFL, flags | O_NONBLOCK)) == -1)
      throw Exception::WithDebug(
          SystemException(GetLastPosixErrorCode()),
          "unable to set non-blocking flag on descriptor");
  }
}

} // namespace posix
} // namespace stp
