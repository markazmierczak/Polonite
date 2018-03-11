// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_DIRECTORYENUMERATOR_H_
#define STP_BASE_FS_DIRECTORYENUMERATOR_H_

#include "Base/Error/SystemErrorCode.h"
#include "Base/FileSystem/FilePath.h"

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#elif OS(POSIX)
#include <dirent.h>
#endif

#define HAVE_SYMLINKS (OS(POSIX))

namespace stp {

class Time;

// A helper for Directory to enumerate the files in provided path.
// The order of the results are not guaranteed.
class DirectoryEnumerator {
  DISALLOW_COPY_AND_ASSIGN(DirectoryEnumerator);
 public:
  DirectoryEnumerator();
  ~DirectoryEnumerator();

  void open(const FilePath& path);
  SystemErrorCode tryOpen(const FilePath& path);

  void open(const FilePath& path, StringSpan pattern);
  SystemErrorCode tryOpen(const FilePath& path, StringSpan pattern);

  void close();

  bool isOpen() const;

  bool tryMoveNext(SystemErrorCode& out_error_code);
  bool moveNext();

  FilePathSpan getFileName() const;

  bool isDirectory() const;

  #if OS(WIN)
  bool isReadOnly() const { return (find_data_.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0; }
  bool isReparsePoint() const { return (find_data_.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0; }
  uint64_t getSize() const;
  Time getLastAccessTime() const;
  Time getLastModifiedTime() const;
  Time getCreationTime() const;
  #elif OS(POSIX)
  bool isRegularFile() const { return dirent_->d_type == DT_REG; }
  bool isSymbolicLink() const { return dirent_->d_type == DT_LNK; }
  #endif

  #if OS(WIN)
  const WIN32_FIND_DATA& getNativeEntry() const { return find_data_; }
  #elif OS(POSIX)
  const struct dirent* getNativeEntry() const { return dirent_; }
  #endif

 private:
  #if OS(WIN)
  enum class Status { Closed, Empty, AtFirst, AtNext };
  WString pattern_;
  FilePath search_path_;
  HANDLE find_handle_ = INVALID_HANDLE_VALUE;
  Status status_ = Status::Closed;
  WIN32_FIND_DATA find_data_;
  #elif OS(POSIX)
  String pattern_;
  struct dirent* dirent_ = nullptr;
  DIR* current_dir_ = nullptr;
  #endif // OS(*)
};

#if OS(WIN)
inline bool DirectoryEnumerator::isOpen() const {
  return status_ != Status::Closed;
}
inline FilePathSpan DirectoryEnumerator::getFileName() const {
  return makeFilePathSpanFromNullTerminated(find_data_.cFileName);
}
inline bool DirectoryEnumerator::isDirectory() const {
  return (find_data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
#elif OS(POSIX)
inline bool DirectoryEnumerator::isOpen() const {
  return current_dir_ != nullptr;
}
inline FilePathSpan DirectoryEnumerator::getFileName() const {
  return makeFilePathSpanFromNullTerminated(dirent_->d_name);
}
inline bool DirectoryEnumerator::isDirectory() const {
  return dirent_->d_type == DT_DIR;
}
#endif // OS(*)

} // namespace stp

#endif // STP_BASE_FS_DIRECTORYENUMERATOR_H_
