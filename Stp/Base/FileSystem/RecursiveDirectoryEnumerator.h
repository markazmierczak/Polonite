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

  void open(FilePath root_path);
  SystemErrorCode tryOpen(FilePath root_path);
  void close();

  bool isOpen() const { return base_.isOpen() || !pending_dir_paths_.isEmpty(); }

  bool tryMoveNext(SystemErrorCode& out_error_code);
  bool moveNext();

  FilePath getEntryFullPath() const;
  const FilePath& getCurrentDirPath() const { return current_dir_path_; }

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
