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

  if (native_.isValid()) {
    if (lifetime_ == AutoClose)
      native_.reset();
    else
      ignoreResult(native_.release());
  }
}

bool FileStream::isOpen() const noexcept {
  return native_.isValid();
}

void FileStream::open(const FilePath& path, FileMode mode, FileAccess access) {
  auto error_code = tryOpen(path, mode, access);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
}

void FileStream::create(const FilePath& path, FileMode mode, FileAccess access) {
  auto error_code = tryCreate(path, mode, access);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
}

SystemErrorCode FileStream::tryOpen(const FilePath& path, FileMode mode, FileAccess access) {
  ASSERT(mode >= FileMode::OpenExisting);
  return tryOpenInternal(path, mode, access);
}

SystemErrorCode FileStream::tryCreate(const FilePath& path, FileMode mode, FileAccess access) {
  ASSERT(mode <= FileMode::CreateNew);
  return tryOpenInternal(path, mode, access);
}

void FileStream::openNative(NativeFile native_file, FileAccess access, NativeFileLifetime lifetime) {
  ASSERT(native_file != InvalidNativeFile);
  ASSERT(!isOpen());

  access_ = access;
  lifetime_ = lifetime;
  native_.reset(native_file);
}

void FileStream::close() {
  ASSERT(isOpen());

  auto nf = native_.release();
  lifetime_ = AutoClose;
  seekable_ = -1;

  closeInternal(nf);
}

bool FileStream::canRead() {
  return isOpen() && access_ != FileAccess::WriteOnly;
}

bool FileStream::canWrite() {
  return isOpen() && access_ != FileAccess::ReadOnly;
}

bool FileStream::canSeek() {
  if (seekable_ < 0) {
    if (!isOpen())
      return false;
    seekable_ = canSeekInternal() ? 1 : 0;
  }
  return seekable_ > 0;
}

void FileStream::flush() {
  // No data is cached by the application, use SyncToDisk() to
  // synchronize the data down to the disk.
}

void FileStream::setPosition(int64_t position) {
  seek(position, SeekOrigin::Begin);
}

int64_t FileStream::getPosition() {
  return seek(0, SeekOrigin::Current);
}

} // namespace stp
