// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Posix/FileDescriptor.h"

#include "Base/Error/SystemException.h"

#include <fcntl.h>

namespace stp {
namespace posix {

FileDescriptor& FileDescriptor::operator=(FileDescriptor&& other) noexcept {
  ASSERT(this != &other);
  FileDescriptor tmp(exchange(fd_, exchange(other.fd_, InvalidFd)));
  return *this;
}

void FileDescriptor::close() {
  ASSERT(isValid());
  // It's important to crash here.
  // There are security implications to not closing a file descriptor properly.
  // As file descriptors are "capabilities", keeping them open would make the
  // current process keep access to a resource.
  int ret = IGNORE_EINTR(::close(fd_));
  if (ret != 0) {
    throw Exception::withDebug(
        SystemException(getLastPosixErrorCode()), "closing descriptor failed");
  }
  fd_ = InvalidFd;
}

FileDescriptor FileDescriptor::duplicate() {
  FileDescriptor rv = tryDuplicate();
  if (!rv.isValid()) {
    throw Exception::withDebug(
        SystemException(getLastPosixErrorCode()), "failed to duplicate file descriptor");
  }
  return rv;
}

FileDescriptor FileDescriptor::duplicateTo(int new_fd) {
  FileDescriptor rv = tryDuplicateTo(new_fd);
  if (!rv.isValid()) {
    throw Exception::withDebug(
        SystemException(getLastPosixErrorCode()), "failed to duplicate file descriptor");
  }
  return rv;
}

void FileDescriptor::setNonBlocking() {
  int flags = ::fcntl(fd_, F_GETFL);

  if (flags == -1) {
    throw Exception::withDebug(
        SystemException(getLastPosixErrorCode()), "unable to get descriptor flags");
  }

  if (!(flags & O_NONBLOCK)) {
    if (HANDLE_EINTR(::fcntl(fd_, F_SETFL, flags | O_NONBLOCK)) == -1)
      throw Exception::withDebug(
          SystemException(getLastPosixErrorCode()),
          "unable to set non-blocking flag on descriptor");
  }
}

} // namespace posix
} // namespace stp
