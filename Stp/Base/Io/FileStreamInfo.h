// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_FILESTREAMINFO_H_
#define STP_BASE_IO_FILESTREAMINFO_H_

#include "Base/FileSystem/FilePath.h"
#include "Base/Time/Time.h"

#if OS(POSIX)
#include "Base/Posix/StatWrapper.h"
#elif OS(WIN)
#include "Base/Win/WindowsHeader.h"
#endif

namespace stp {

class FileStreamInfo {
  DISALLOW_COPY_AND_ASSIGN(FileStreamInfo);
 public:
  FileStreamInfo() = default;

  uint64_t getSize() const;

  Time getLastAccessTime() const;
  Time getLastModifiedTime() const;
  #if OS(WIN)
  Time getCreationTime() const;
  #endif

  bool isDirectory() const;
  #if OS(POSIX)
  bool isSymbolicLink() const;
  #endif

 private:
  friend class FileStream;

  #if OS(WIN)
  BY_HANDLE_FILE_INFORMATION by_handle_;
  #elif OS(POSIX)
  stat_wrapper_t stat_;
  #endif
};

#if OS(WIN)
inline bool FileStreamInfo::isRegularFile() const {
  // FIXME support for symlinks
  return !isDirectory();
}
inline bool FileStreamInfo::isDirectory() const {
  return (by_handle_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

inline uint64_t FileStreamInfo::getSize() const {
  ULARGE_INTEGER size;
  size.HighPart = by_handle_.nFileSizeHigh;
  size.LowPart  = by_handle_.nFileSizeLow;
  return size.QuadPart;
}

inline Time FileStreamInfo::getLastAccessTime() const {
  return Time::fromFileTime(by_handle_.ftLastAccessTime);
}
inline Time FileStreamInfo::getLastModifiedTime() const {
  return Time::fromFileTime(by_handle_.ftLastWriteTime);
}
inline Time FileStreamInfo::getCreationTime() const {
  return Time::fromFileTime(by_handle_.ftCreationTime);
}
#elif OS(POSIX)
inline uint64_t FileStreamInfo::getSize() const {
  return stat_.st_size;
}
inline bool FileStreamInfo::isDirectory() const {
  return S_ISDIR(stat_.st_mode);
}
inline bool FileStreamInfo::isSymbolicLink() const {
  return S_ISLNK(stat_.st_mode);
}

inline Time FileStreamInfo::getLastAccessTime() const {
  return Time::FromTimeT(stat_.st_atime);
}
inline Time FileStreamInfo::getLastModifiedTime() const {
  return Time::FromTimeT(stat_.st_mtime);
}
#endif

} // namespace stp

#endif // STP_BASE_IO_FILESTREAMINFO_H_
