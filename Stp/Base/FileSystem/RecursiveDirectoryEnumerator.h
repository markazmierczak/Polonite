// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_RECURSIVEDIRECTORYENUMERATOR_H_
#define STP_BASE_FS_RECURSIVEDIRECTORYENUMERATOR_H_

#include "Base/Containers/Stack.h"
#include "Base/FileSystem/DirectoryEnumerator.h"

namespace stp {

class RecursiveDirectoryEnumerator {
 public:
  RecursiveDirectoryEnumerator();
  ~RecursiveDirectoryEnumerator();

  void Open(FilePath root_path);
  ErrorCode TryOpen(FilePath root_path);

  bool IsOpen() const { return base_.IsOpen() || !pending_dir_paths_.IsEmpty(); }

  void Close();

  bool TryMoveNext(ErrorCode& out_error_code);
  bool MoveNext();

  FilePath GetEntryFullPath() const;
  const FilePath& GetCurrentDirPath() const { return current_dir_path_; }

  const DirectoryEnumerator& base() const { return base_; }

 private:
  DirectoryEnumerator base_;

  FilePath current_dir_path_;

  // A stack that keeps track of which sub-directories we still need to
  // enumerate in the breadth-first search.
  Stack<FilePath> pending_dir_paths_;
};

} // namespace stp

#endif // STP_BASE_FS_RECURSIVEDIRECTORYENUMERATOR_H_
