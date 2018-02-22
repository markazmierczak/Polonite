// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_TEMPORARYDIRECTORY_H_
#define STP_BASE_FS_TEMPORARYDIRECTORY_H_

#include "Base/FileSystem/FilePath.h"
#include "Base/FileSystem/FileSystemException.h"

namespace stp {

class BASE_EXPORT TemporaryDirectory {
  DISALLOW_COPY_AND_ASSIGN(TemporaryDirectory);
 public:
  TemporaryDirectory();
  ~TemporaryDirectory();

  bool isValid() const { return !path_.isEmpty(); }

  void create();
  void createUnder(const FilePath& path);
  void create(FilePath path);

  void remove();

  FilePath take() { return move(path_); }

  ALWAYS_INLINE const FilePath& path() const { return path_; }

 private:
  FilePath path_;

  void createInternal(FilePathSpan dir, StringSpan prefix);
};

} // namespace stp

#endif // STP_BASE_FS_TEMPORARYDIRECTORY_H_
