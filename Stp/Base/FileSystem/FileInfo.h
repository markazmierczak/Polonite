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
  DISALLOW_COPY_AND_ASSIGN(FileInfo);
 public:
  FileInfo() = default;

  uint64_t getSize() const;

  Time lastAccessTime() const;
  Time lastModifiedTime() const;
  #if OS(WIN)
  Time getCreationTime() const;
  #endif

  bool isDirectory() const;
  #if OS(WIN)
  bool isReadOnly() const { return (attr_data_.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0; }
  #elif OS(POSIX)
  bool isSymbolicLink() const { return S_ISLNK(stat_.st_mode); }
  #endif

 private:
  friend class DirectoryEnumerator;
  friend class File;

  #if OS(WIN)
  WIN32_FILE_ATTRIBUTE_DATA attr_data_;
  #elif OS(POSIX)
  stat_wrapper_t stat_;
  #endif
};

#if OS(WIN)
inline bool FileInfo::isDirectory() const {
  return (attr_data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

inline uint64_t FileInfo::getSize() const {
  ULARGE_INTEGER size;
  size.HighPart = attr_data_.nFileSizeHigh;
  size.LowPart  = attr_data_.nFileSizeLow;
  return size.QuadPart;
}

inline Time FileInfo::lastAccessTime() const {
  return Time::fromFileTime(attr_data_.ftLastAccessTime);
}
inline Time FileInfo::lastModifiedTime() const {
  return Time::fromFileTime(attr_data_.ftLastWriteTime);
}
inline Time FileInfo::getCreationTime() const {
  return Time::fromFileTime(attr_data_.ftCreationTime);
}
#elif OS(POSIX)
inline uint64_t FileInfo::getSize() const {
  return stat_.st_size;
}
inline bool FileInfo::isDirectory() const {
  return S_ISDIR(stat_.st_mode);
}

inline Time FileInfo::lastAccessTime() const {
  return Time::FromTimeT(stat_.st_atime);
}
inline Time FileInfo::lastModifiedTime() const {
  return Time::FromTimeT(stat_.st_mtime);
}
#endif // OS(*)

} // namespace stp

#endif // STP_BASE_FS_FILEINFO_H_
