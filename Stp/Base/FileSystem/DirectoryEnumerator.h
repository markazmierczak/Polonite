// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_DIRECTORYENUMERATOR_H_
#define STP_BASE_FS_DIRECTORYENUMERATOR_H_

#include "Base/Error/ErrorCode.h"
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
 public:
  DirectoryEnumerator();
  ~DirectoryEnumerator();

  void Open(const FilePath& path);
  ErrorCode TryOpen(const FilePath& path);

  void Open(const FilePath& path, StringSpan pattern);
  ErrorCode TryOpen(const FilePath& path, StringSpan pattern);

  void Close() noexcept;

  bool IsOpen() const;

  bool TryMoveNext(ErrorCode& out_error_code);
  bool MoveNext();

  FilePathSpan GetFileName() const;

  bool IsDirectory() const;

  #if OS(WIN)
  bool IsReadOnly() const { return (find_data_.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0; }
  bool IsReparsePoint() const { return (find_data_.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0; }
  uint64_t GetSize() const;
  Time GetLastAccessTime() const;
  Time GetLastModifiedTime() const;
  Time GetCreationTime() const;
  #elif OS(POSIX)
  bool IsRegularFile() const { return dirent_->d_type == DT_REG; }
  bool IsSymbolicLink() const { return dirent_->d_type == DT_LNK; }
  #endif

  #if OS(WIN)
  const WIN32_FIND_DATA& GetNativeEntry() const { return find_data_; }
  #elif OS(POSIX)
  const struct dirent* GetNativeEntry() const { return dirent_; }
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

  DISALLOW_COPY_AND_ASSIGN(DirectoryEnumerator);
};

#if OS(WIN)
inline bool DirectoryEnumerator::IsOpen() const {
  return status_ != Status::Closed;
}
inline FilePathSpan DirectoryEnumerator::GetFileName() const {
  return MakeFilePathSpanFromNullTerminated(find_data_.cFileName);
}
inline bool DirectoryEnumerator::IsDirectory() const {
  return (find_data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
#elif OS(POSIX)
inline bool DirectoryEnumerator::IsOpen() const {
  return current_dir_ != nullptr;
}
inline FilePathSpan DirectoryEnumerator::GetFileName() const {
  return MakeFilePathSpanFromNullTerminated(dirent_->d_name);
}
inline bool DirectoryEnumerator::IsDirectory() const {
  return dirent_->d_type == DT_DIR;
}
#endif // OS(*)

} // namespace stp

#endif // STP_BASE_FS_DIRECTORYENUMERATOR_H_
