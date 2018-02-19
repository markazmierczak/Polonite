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

SystemErrorCode FileStream::TryOpenInternal(const FilePath& path, FileMode mode, FileAccess access) {
  ASSERT(!IsOpen());

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
    return GetLastWinErrorCode();

  native_.Reset(handle);
  access_ = access;
  #if ASSERT_IS_ON
  append_ = mode == FileMode::Append;
  #endif
  return SystemErrorCode::Success;
}

void FileStream::CloseInternal(NativeFile handle) {
  if (!::CloseHandle(handle))
    throw SystemException(GetLastWinErrorCode());
}

int FileStream::ReadAtMost(MutableBufferSpan output) {
  ASSERT(CanRead());
  DWORD bytes_read;
  if (!::ReadFile(native_.get(), output.data(), output.size(), &bytes_read, NULL))
    throw SystemException(GetLastWinErrorCode());
  return bytes_read;
}

void FileStream::Write(BufferSpan input) {
  ASSERT(CanWrite());
  DWORD bytes_written;
  if (!::WriteFile(native_.get(), input.data(), input.size(), &bytes_written, NULL))
    throw SystemException(GetLastWinErrorCode());
}

void FileStream::PositionalRead(int64_t offset, MutableBufferSpan output) {
  ASSERT(offset >= 0);
  ASSERT(CanRead() && CanSeek());

  LARGE_INTEGER offset_li;
  offset_li.QuadPart = offset;

  OVERLAPPED overlapped = {0};
  overlapped.Offset = offset_li.LowPart;
  overlapped.OffsetHigh = offset_li.HighPart;

  if (!::ReadFile(native_.get(), output.data(), output.size(), NULL, &overlapped))
    throw SystemException(GetLastWinErrorCode());
}

void FileStream::PositionalWrite(int64_t offset, BufferSpan input) {
  ASSERT(offset >= 0);
  ASSERT(CanWrite() && CanSeek());
  ASSERT(!append_);

  LARGE_INTEGER offset_li;
  offset_li.QuadPart = offset;

  OVERLAPPED overlapped = {0};
  overlapped.Offset = offset_li.LowPart;
  overlapped.OffsetHigh = offset_li.HighPart;

  if (!::WriteFile(native_.get(), input.data(), input.size(), NULL, &overlapped))
    throw SystemException(GetLastWinErrorCode());
}

int64_t FileStream::Seek(int64_t offset, SeekOrigin origin) {
  ASSERT(CanSeek());
  LARGE_INTEGER distance, res;
  distance.QuadPart = offset;
  DWORD move_method = static_cast<DWORD>(origin);
  if (!::SetFilePointerEx(native_.get(), distance, &res, move_method))
    throw SystemException(GetLastWinErrorCode());
  return res.QuadPart;
}

bool FileStream::CanSeekInternal() {
  ASSERT(IsOpen());
  DWORD type = ::GetFileType(native_.get());
  return !(type == FILE_TYPE_CHAR || type == FILE_TYPE_PIPE);
}

int64_t FileStream::GetLength() {
  ASSERT(IsOpen());
  LARGE_INTEGER size;
  if (!::GetFileSizeEx(native_.get(), &size))
    throw SystemException(GetLastWinErrorCode());
  return size.QuadPart;
}

void FileStream::SetLength(int64_t length) {
  ASSERT(length >= 0);
  ASSERT(IsOpen());
  SetPosition(length);
  if (!::SetEndOfFile(native_.get()))
    throw SystemException(GetLastWinErrorCode());
}

void FileStream::GetInfo(FileStreamInfo& out) {
  ASSERT(IsOpen());
  if (!::GetFileInformationByHandle(native_.get(), &out.by_handle_))
    throw SystemException(GetLastWinErrorCode());
}

static FILETIME* TimeToNullableFiletime(Time time, FILETIME& storage) {
  if (time.isNull())
    return nullptr;
  storage = time.ToFileTime();
  return &storage;
}

void FileStream::SetTimes(Time last_accessed, Time last_modified, Time creation_time) {
  ASSERT(IsOpen());
  FILETIME storage[3];
  auto* last_accessed_ft = TimeToNullableFiletime(last_accessed, storage[0]);
  auto* last_modified_ft = TimeToNullableFiletime(last_modified, storage[1]);
  auto* creation_time_ft = TimeToNullableFiletime(creation_time, storage[2]);

  if (!::SetFileTime(native_.get(), creation_time_ft, last_accessed_ft, last_modified_ft))
    throw SystemException(GetLastWinErrorCode());
}

void FileStream::SyncToDisk() {
  ASSERT(IsOpen());
  if (!::FlushFileBuffers(native_.get()))
    throw SystemException(GetLastWinErrorCode());
}

} // namespace stp
