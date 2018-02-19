// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_FS_TEMPORARYDIRECTORY_H_
#define STP_BASE_FS_TEMPORARYDIRECTORY_H_

#include "Base/FileSystem/FilePath.h"
#include "Base/FileSystem/FileSystemException.h"

namespace stp {

// An object representing a temporary / scratch directory that should be cleaned
// up (recursively) when this object is destroyed.
class BASE_EXPORT TemporaryDirectory {
 public:
  // No directory is owned/created initially.
  TemporaryDirectory();

  // Recursively delete path.
  ~TemporaryDirectory();

  // Returns true if path_ is non-empty.
  bool IsValid() const { return !path_.isEmpty(); }

  // Creates a unique directory in system-wide temporary directory,
  // and takes ownership of it.
  void Create();

  // Creates a unique directory under a given path, and takes ownership of it.
  void CreateUnder(const FilePath& path);

  // Takes ownership of directory at |path|, creating it if necessary.
  // Don't call multiple times unless Take() has been called first.
  void Create(FilePath path);

  // Deletes the temporary directory wrapped by this object.
  void Remove();

  // Caller takes ownership of the temporary directory so it won't be destroyed
  // when this object goes out of scope.
  FilePath Take() { return move(path_); }

  ALWAYS_INLINE const FilePath& path() const { return path_; }

 private:
  void CreateInternal(FilePathSpan dir, StringSpan prefix);

  FilePath path_;

  DISALLOW_COPY_AND_ASSIGN(TemporaryDirectory);
};

} // namespace stp

#endif // STP_BASE_FS_TEMPORARYDIRECTORY_H_
