// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/RecursiveDirectoryEnumerator.h"

#include "Base/FileSystem/FileSystemException.h"

namespace stp {

RecursiveDirectoryEnumerator::RecursiveDirectoryEnumerator() {}
RecursiveDirectoryEnumerator::~RecursiveDirectoryEnumerator() {}

SystemErrorCode RecursiveDirectoryEnumerator::tryOpen(FilePath root_path) {
  ASSERT(!isOpen());
  current_dir_path_ = move(root_path);
  return base_.tryOpen(root_path);
}

void RecursiveDirectoryEnumerator::open(FilePath root_path) {
  auto error_code = tryOpen(move(root_path));
  if (!isOk(error_code))
    throw FileSystemException(error_code, current_dir_path_);
}

void RecursiveDirectoryEnumerator::close() {
  ASSERT(isOpen());
  base_.close();
  current_dir_path_.clear();
  pending_dir_paths_.clear();
}

bool RecursiveDirectoryEnumerator::tryMoveNext(SystemErrorCode& out_error_code) {
  ASSERT(isOpen());
  while (true) {
    if (base_.isOpen()) {
      if (base_.tryMoveNext(out_error_code)) {
        if (base_.isDirectory())
          pending_dir_paths_.push(combineFilePaths(current_dir_path_, base_.getFileName()));
        return true;
      }
      base_.close();
      if (!isOk(out_error_code))
        return false;
    }
    if (pending_dir_paths_.isEmpty()) {
      out_error_code = SystemErrorCode::Ok;
      break;
    }
    current_dir_path_ = pending_dir_paths_.pop();
    out_error_code = base_.tryOpen(current_dir_path_);
  }
  return false;
}

bool RecursiveDirectoryEnumerator::moveNext() {
  SystemErrorCode error_code;
  bool has_next = tryMoveNext(error_code);
  if (!has_next && !isOk(error_code))
    throw FileSystemException(error_code, current_dir_path_);
  return has_next;
}

FilePath RecursiveDirectoryEnumerator::getEntryFullPath() const {
  return combineFilePaths(current_dir_path_, base().getFileName());
}

} // namespace stp
