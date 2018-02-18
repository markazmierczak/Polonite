// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/FileStream.h"

#include "Base/FileSystem/FileSystemException.h"

namespace stp {

FileStream::FileStream(NativeFile native_file, FileAccess access)
    : native_(native_file),
      access_(access) {
}

FileStream::~FileStream() {
  // NOTE: Do not flush here. It may cause slow down.
  // User of this API must call Flush() manually instead.

  if (native_.IsValid()) {
    if (lifetime_ == AutoClose)
      native_.Reset();
    else
      ignoreResult(native_.release());
  }
}

bool FileStream::IsOpen() const noexcept {
  return native_.IsValid();
}

void FileStream::Open(const FilePath& path, FileMode mode, FileAccess access) {
  auto error_code = TryOpen(path, mode, access);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
}

void FileStream::Create(const FilePath& path, FileMode mode, FileAccess access) {
  auto error_code = TryCreate(path, mode, access);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
}

SystemErrorCode FileStream::TryOpen(const FilePath& path, FileMode mode, FileAccess access) {
  ASSERT(mode >= FileMode::OpenExisting);
  return TryOpenInternal(path, mode, access);
}

SystemErrorCode FileStream::TryCreate(const FilePath& path, FileMode mode, FileAccess access) {
  ASSERT(mode <= FileMode::CreateNew);
  return TryOpenInternal(path, mode, access);
}

void FileStream::OpenNative(NativeFile native_file, FileAccess access, NativeFileLifetime lifetime) {
  ASSERT(native_file != InvalidNativeFile);
  ASSERT(!IsOpen());

  access_ = access;
  lifetime_ = lifetime;
  native_.Reset(native_file);
}

void FileStream::Close() {
  ASSERT(IsOpen());

  auto nf = native_.release();
  lifetime_ = AutoClose;
  seekable_ = -1;

  CloseInternal(nf);
}

bool FileStream::CanRead() {
  return IsOpen() && access_ != FileAccess::WriteOnly;
}

bool FileStream::CanWrite() {
  return IsOpen() && access_ != FileAccess::ReadOnly;
}

bool FileStream::CanSeek() {
  if (seekable_ < 0) {
    if (!IsOpen())
      return false;
    seekable_ = CanSeekInternal() ? 1 : 0;
  }
  return seekable_ > 0;
}

void FileStream::Flush() {
  // No data is cached by the application, use SyncToDisk() to
  // synchronize the data down to the disk.
}

void FileStream::SetPosition(int64_t position) {
  Seek(position, SeekOrigin::Begin);
}

int64_t FileStream::GetPosition() {
  return Seek(0, SeekOrigin::Current);
}

} // namespace stp
