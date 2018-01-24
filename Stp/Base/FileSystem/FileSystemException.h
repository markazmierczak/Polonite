// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_FILESYSTEMEXCEPTION_H_
#define STP_BASE_FS_FILESYSTEMEXCEPTION_H_

#include "Base/FileSystem/FilePath.h"
#include "Base/Error/SystemException.h"

namespace stp {

class BASE_EXPORT FileSystemException : public SystemException {
 public:
  explicit FileSystemException(const ErrorCode& error_code) noexcept
      : SystemException(error_code) {}

  FileSystemException(const ErrorCode& error_code, FilePathSpan path)
      : SystemException(error_code), path_(path) {}

  FileSystemException(const ErrorCode& error_code, FilePathSpan path, FilePathSpan aux_path)
      : SystemException(error_code), path_(path), aux_path_(aux_path) {}

  const FilePath& GetPath() const { return path_; }
  const FilePath& GetAuxPath() const { return aux_path_; }

  StringSpan GetName() const noexcept override;

 protected:
  void OnFormat(TextWriter& out) const final;

 private:
  FilePath path_;
  FilePath aux_path_;
};

} // namespace stp

#endif // STP_BASE_FS_FILESYSTEMEXCEPTION_H_
