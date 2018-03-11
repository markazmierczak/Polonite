// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_POSIX_FILEDESCRIPTOR_H_
#define STP_BASE_POSIX_FILEDESCRIPTOR_H_

#include "Base/Containers/BufferSpan.h"
#include "Base/Posix/EintrWrapper.h"

#include <sys/types.h>
#include <unistd.h>

namespace stp {
namespace posix {

class BASE_EXPORT FileDescriptor {
 public:
  FileDescriptor() = default;
  ~FileDescriptor();

  FileDescriptor(FileDescriptor&& other) : fd_(other.leakDescriptor()) {}
  FileDescriptor& operator=(FileDescriptor&& other);

  explicit FileDescriptor(int fd) : fd_(fd) {}

  [[nodiscard]] int leakDescriptor() { return exchange(fd_, InvalidFd); }

  void reset(int new_fd = InvalidFd);

  ALWAYS_INLINE int get() const { return fd_; }
  ALWAYS_INLINE bool isValid() const { return fd_ != InvalidFd; }

  [[nodiscard]] FileDescriptor duplicate();
  [[nodiscard]] FileDescriptor tryDuplicate() { return FileDescriptor(::dup(fd_)); }

  [[nodiscard]] FileDescriptor tryDuplicateTo(int new_fd) {
    return FileDescriptor(IGNORE_EINTR(::dup2(fd_, new_fd)));
  }
  [[nodiscard]] FileDescriptor duplicateTo(int new_fd);

  void setNonBlocking();

  // Read carefully POSIX documentation on what values are returned
  // from read()/write() syscalls. For example POSIX allows a read() that is
  // interrupted after reading some data to return -1 (with errno set to EINTR) or
  // to return the number of bytes already read.
  // Thus these methods are marked as "no best-effort".
  int tryReadNoBestEffort(MutableBufferSpan buffer);
  int tryWriteNoBestEffort(BufferSpan buffer);

  friend void swap(FileDescriptor& l, FileDescriptor& r) { swap(l.fd_, r.fd_); }

 private:
  static constexpr int InvalidFd = -1;

  void close();

  int fd_ = InvalidFd;

  DISALLOW_COPY_AND_ASSIGN(FileDescriptor);
};

inline FileDescriptor::~FileDescriptor() {
  if (isValid())
    close();
}

inline void FileDescriptor::reset(int new_fd) {
  FileDescriptor tmp(new_fd);
  swap(*this, tmp);
}

inline int FileDescriptor::tryReadNoBestEffort(MutableBufferSpan buffer) {
  ASSERT(isValid());
  ssize_t rv = HANDLE_EINTR(::read(fd_, buffer.data(), toUnsigned(buffer.size())));
  ASSERT(-1 <= rv && rv <= buffer.size());
  return static_cast<int>(rv);
}

inline int FileDescriptor::tryWriteNoBestEffort(BufferSpan buffer) {
  ASSERT(isValid());
  ssize_t rv = HANDLE_EINTR(::write(fd_, buffer.data(), toUnsigned(buffer.size())));
  ASSERT(-1 <= rv && rv <= buffer.size());
  return static_cast<int>(rv);
}

} // namespace posix
} // namespace stp

#endif // STP_BASE_POSIX_FILEDESCRIPTOR_H_
