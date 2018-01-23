// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_FILESTREAMINFO_H_
#define STP_BASE_IO_FILESTREAMINFO_H_

#include "Base/Fs/FilePath.h"
#include "Base/Time/Time.h"

#if OS(POSIX)
#include "Base/Posix/StatWrapper.h"
#elif OS(WIN)
#include "Base/Win/WindowsHeader.h"
#endif

namespace stp {

class FileStreamInfo {
 public:
  FileStreamInfo() = default;

  uint64_t GetSize() const;

  Time GetLastAccessTime() const;
  Time GetLastModifiedTime() const;
  #if OS(WIN)
  Time GetCreationTime() const;
  #endif

  bool IsDirectory() const;
  #if OS(POSIX)
  bool IsSymbolicLink() const;
  #endif

 private:
  friend class FileStream;

  #if OS(WIN)
  BY_HANDLE_FILE_INFORMATION by_handle_;
  #elif OS(POSIX)
  stat_wrapper_t stat_;
  #endif

  DISALLOW_COPY_AND_ASSIGN(FileStreamInfo);
};

#if OS(WIN)
inline bool FileStreamInfo::IsRegularFile() const {
  // FIXME support for symlinks
  return !IsDirectory();
}
inline bool FileStreamInfo::IsDirectory() const {
  return (by_handle_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

inline uint64_t FileStreamInfo::GetSize() const {
  ULARGE_INTEGER size;
  size.HighPart = by_handle_.nFileSizeHigh;
  size.LowPart  = by_handle_.nFileSizeLow;
  return size.QuadPart;
}

inline Time FileStreamInfo::GetLastAccessTime() const {
  return Time::FromFileTime(by_handle_.ftLastAccessTime);
}
inline Time FileStreamInfo::GetLastModifiedTime() const {
  return Time::FromFileTime(by_handle_.ftLastWriteTime);
}
inline Time FileStreamInfo::GetCreationTime() const {
  return Time::FromFileTime(by_handle_.ftCreationTime);
}
#elif OS(POSIX)
inline uint64_t FileStreamInfo::GetSize() const {
  return stat_.st_size;
}
inline bool FileStreamInfo::IsDirectory() const {
  return S_ISDIR(stat_.st_mode);
}
inline bool FileStreamInfo::IsSymbolicLink() const {
  return S_ISLNK(stat_.st_mode);
}

inline Time FileStreamInfo::GetLastAccessTime() const {
  return Time::FromTimeT(stat_.st_atime);
}
inline Time FileStreamInfo::GetLastModifiedTime() const {
  return Time::FromTimeT(stat_.st_mtime);
}
#endif

} // namespace stp

#endif // STP_BASE_IO_FILESTREAMINFO_H_
