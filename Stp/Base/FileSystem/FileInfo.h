// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_FILEINFO_H_
#define STP_BASE_FS_FILEINFO_H_

#include "Base/FileSystem/FilePath.h"
#include "Base/Time/Time.h"

#if OS(POSIX)
#include "Base/Posix/StatWrapper.h"
#elif OS(WIN)
#include "Base/Win/WindowsHeader.h"
#endif

namespace stp {

class FileInfo {
 public:
  FileInfo() = default;

  uint64_t GetSize() const;

  Time GetLastAccessTime() const;
  Time GetLastModifiedTime() const;
  #if OS(WIN)
  Time GetCreationTime() const;
  #endif

  bool IsDirectory() const;
  #if OS(WIN)
  bool IsReadOnly() const { return (attr_data_.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0; }
  #elif OS(POSIX)
  bool IsSymbolicLink() const { return S_ISLNK(stat_.st_mode); }
  #endif

 private:
  friend class DirectoryEnumerator;
  friend class File;

  #if OS(WIN)
  WIN32_FILE_ATTRIBUTE_DATA attr_data_;
  #elif OS(POSIX)
  stat_wrapper_t stat_;
  #endif

  DISALLOW_COPY_AND_ASSIGN(FileInfo);
};

#if OS(WIN)
inline bool FileInfo::IsDirectory() const {
  return (attr_data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

inline uint64_t FileInfo::GetSize() const {
  ULARGE_INTEGER size;
  size.HighPart = attr_data_.nFileSizeHigh;
  size.LowPart  = attr_data_.nFileSizeLow;
  return size.QuadPart;
}

inline Time FileInfo::GetLastAccessTime() const {
  return Time::FromFileTime(attr_data_.ftLastAccessTime);
}
inline Time FileInfo::GetLastModifiedTime() const {
  return Time::FromFileTime(attr_data_.ftLastWriteTime);
}
inline Time FileInfo::GetCreationTime() const {
  return Time::FromFileTime(attr_data_.ftCreationTime);
}
#elif OS(POSIX)
inline uint64_t FileInfo::GetSize() const {
  return stat_.st_size;
}
inline bool FileInfo::IsDirectory() const {
  return S_ISDIR(stat_.st_mode);
}

inline Time FileInfo::GetLastAccessTime() const {
  return Time::FromTimeT(stat_.st_atime);
}
inline Time FileInfo::GetLastModifiedTime() const {
  return Time::FromTimeT(stat_.st_mtime);
}
#endif // OS(*)

} // namespace stp

#endif // STP_BASE_FS_FILEINFO_H_
