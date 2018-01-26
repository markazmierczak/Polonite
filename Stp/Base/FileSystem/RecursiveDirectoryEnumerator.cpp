// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/RecursiveDirectoryEnumerator.h"

#include "Base/FileSystem/FileSystemException.h"

namespace stp {

RecursiveDirectoryEnumerator::RecursiveDirectoryEnumerator() {}
RecursiveDirectoryEnumerator::~RecursiveDirectoryEnumerator() {}

SystemErrorCode RecursiveDirectoryEnumerator::TryOpen(FilePath root_path) {
  ASSERT(!IsOpen());
  current_dir_path_ = Move(root_path);
  return base_.TryOpen(root_path);
}

void RecursiveDirectoryEnumerator::Open(FilePath root_path) {
  auto error_code = TryOpen(Move(root_path));
  if (!IsOk(error_code))
    throw FileSystemException(error_code, current_dir_path_);
}

void RecursiveDirectoryEnumerator::Close() {
  ASSERT(IsOpen());
  base_.Close();
  current_dir_path_.Clear();
  pending_dir_paths_.Clear();
}

bool RecursiveDirectoryEnumerator::TryMoveNext(SystemErrorCode& out_error_code) {
  ASSERT(IsOpen());
  while (true) {
    if (base_.IsOpen()) {
      if (base_.TryMoveNext(out_error_code)) {
        if (base_.IsDirectory())
          pending_dir_paths_.Push(CombineFilePaths(current_dir_path_, base_.GetFileName()));
        return true;
      }
      base_.Close();
      if (!IsOk(out_error_code))
        return false;
    }
    if (pending_dir_paths_.IsEmpty()) {
      out_error_code = SystemErrorCode::Ok;
      break;
    }
    current_dir_path_ = pending_dir_paths_.Pop();
    out_error_code = base_.TryOpen(current_dir_path_);
  }
  return false;
}

bool RecursiveDirectoryEnumerator::MoveNext() {
  SystemErrorCode error_code;
  bool has_next = TryMoveNext(error_code);
  if (!has_next && !IsOk(error_code))
    throw FileSystemException(error_code, current_dir_path_);
  return has_next;
}

FilePath RecursiveDirectoryEnumerator::GetEntryFullPath() const {
  return CombineFilePaths(current_dir_path_, base().GetFileName());
}

} // namespace stp
