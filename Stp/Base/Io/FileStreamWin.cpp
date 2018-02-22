// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/FileStream.h"

#include "Base/FileSystem/FileSystemException.h"
#include "Base/Io/FileStreamInfo.h"
#include "Base/Util/Finally.h"

namespace stp {

static_assert(static_cast<int>(SeekOrigin::Begin) == FILE_BEGIN &&
              static_cast<int>(SeekOrigin::Current) == FILE_CURRENT &&
              static_cast<int>(SeekOrigin::End) == FILE_END,
              "SeekOrigin must match the system headers");

SystemErrorCode FileStream::tryOpenInternal(const FilePath& path, FileMode mode, FileAccess access) {
  ASSERT(!isOpen());

  DWORD disposition = 0;
  switch (mode) {
    case FileMode::Create:
      disposition = CREATE_ALWAYS;
      break;
    case FileMode::CreateNew:
      disposition = CREATE_NEW;
      break;
    case FileMode::OpenExisting:
      disposition = OPEN_EXISTING;
      break;
    case FileMode::OpenTruncated:
      disposition = TRUNCATE_EXISTING;
      break;
    case FileMode::OpenOrCreate:
      disposition = OPEN_ALWAYS;
      break;
    case FileMode::Append:
      disposition = OPEN_EXISTING;
      break;
  }

  DWORD desired_access = 0;
  if (mode == FileMode::Append) {
    ASSERT(access == FileAccess::WriteOnly);
    desired_access = FILE_APPEND_DATA;
  }

  switch (access) {
    case FileAccess::ReadOnly:
      desired_access |= GENERIC_READ;
      break;
    case FileAccess::WriteOnly:
      desired_access |= GENERIC_WRITE;
      break;
    case FileAccess::ReadWrite:
      desired_access |= GENERIC_READ | GENERIC_WRITE;
      break;
  }

  DWORD sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;

  DWORD create_flags = 0;
  NativeFile handle = ::CreateFileW(
      toNullTerminated(path), desired_access, sharing, NULL, disposition, create_flags, NULL);

  if (handle == INVALID_HANDLE_VALUE)
    return getLastWinErrorCode();

  native_.reset(handle);
  access_ = access;
  #if ASSERT_IS_ON
  append_ = mode == FileMode::Append;
  #endif
  return SystemErrorCode::Success;
}

void FileStream::closeInternal(NativeFile handle) {
  if (!::CloseHandle(handle))
    throw SystemException(getLastWinErrorCode());
}

int FileStream::readAtMost(MutableBufferSpan output) {
  ASSERT(canRead());
  DWORD bytes_read;
  if (!::ReadFile(native_.get(), output.data(), output.size(), &bytes_read, NULL))
    throw SystemException(getLastWinErrorCode());
  return bytes_read;
}

void FileStream::write(BufferSpan input) {
  ASSERT(canWrite());
  DWORD bytes_written;
  if (!::WriteFile(native_.get(), input.data(), input.size(), &bytes_written, NULL))
    throw SystemException(getLastWinErrorCode());
}

void FileStream::positionalRead(int64_t offset, MutableBufferSpan output) {
  ASSERT(offset >= 0);
  ASSERT(canRead() && canSeek());

  LARGE_INTEGER offset_li;
  offset_li.QuadPart = offset;

  OVERLAPPED overlapped = {0};
  overlapped.Offset = offset_li.LowPart;
  overlapped.OffsetHigh = offset_li.HighPart;

  if (!::ReadFile(native_.get(), output.data(), output.size(), NULL, &overlapped))
    throw SystemException(getLastWinErrorCode());
}

void FileStream::positionalWrite(int64_t offset, BufferSpan input) {
  ASSERT(offset >= 0);
  ASSERT(canWrite() && canSeek());
  ASSERT(!append_);

  LARGE_INTEGER offset_li;
  offset_li.QuadPart = offset;

  OVERLAPPED overlapped = {0};
  overlapped.Offset = offset_li.LowPart;
  overlapped.OffsetHigh = offset_li.HighPart;

  if (!::WriteFile(native_.get(), input.data(), input.size(), NULL, &overlapped))
    throw SystemException(getLastWinErrorCode());
}

int64_t FileStream::seek(int64_t offset, SeekOrigin origin) {
  ASSERT(canSeek());
  LARGE_INTEGER distance, res;
  distance.QuadPart = offset;
  DWORD move_method = static_cast<DWORD>(origin);
  if (!::SetFilePointerEx(native_.get(), distance, &res, move_method))
    throw SystemException(getLastWinErrorCode());
  return res.QuadPart;
}

bool FileStream::canSeekInternal() {
  ASSERT(isOpen());
  DWORD type = ::GetFileType(native_.get());
  return !(type == FILE_TYPE_CHAR || type == FILE_TYPE_PIPE);
}

int64_t FileStream::getLength() {
  ASSERT(isOpen());
  LARGE_INTEGER size;
  if (!::GetFileSizeEx(native_.get(), &size))
    throw SystemException(getLastWinErrorCode());
  return size.QuadPart;
}

void FileStream::setLength(int64_t length) {
  ASSERT(length >= 0);
  ASSERT(isOpen());
  setPosition(length);
  if (!::SetEndOfFile(native_.get()))
    throw SystemException(getLastWinErrorCode());
}

void FileStream::getInfo(FileStreamInfo& out) {
  ASSERT(isOpen());
  if (!::GetFileInformationByHandle(native_.get(), &out.by_handle_))
    throw SystemException(getLastWinErrorCode());
}

static FILETIME* TimeToNullableFiletime(Time time, FILETIME& storage) {
  if (time.isNull())
    return nullptr;
  storage = time.ToFileTime();
  return &storage;
}

void FileStream::setTimes(Time last_accessed, Time last_modified, Time creation_time) {
  ASSERT(isOpen());
  FILETIME storage[3];
  auto* last_accessed_ft = TimeToNullableFiletime(last_accessed, storage[0]);
  auto* last_modified_ft = TimeToNullableFiletime(last_modified, storage[1]);
  auto* creation_time_ft = TimeToNullableFiletime(creation_time, storage[2]);

  if (!::SetFileTime(native_.get(), creation_time_ft, last_accessed_ft, last_modified_ft))
    throw SystemException(getLastWinErrorCode());
}

void FileStream::syncToDisk() {
  ASSERT(isOpen());
  if (!::FlushFileBuffers(native_.get()))
    throw SystemException(getLastWinErrorCode());
}

} // namespace stp
