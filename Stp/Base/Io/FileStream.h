// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_FILESTREAM_H_
#define STP_BASE_IO_FILESTREAM_H_

#include "Base/FileSystem/File.h"
#include "Base/Io/Stream.h"

#if OS(POSIX)
#include "Base/Posix/FileDescriptor.h"
#endif

namespace stp {

class Time;
class FileStreamInfo;

class BASE_EXPORT FileStream final : public Stream {
 public:
  FileStream() {}
  ~FileStream() override;

  void Create(
      const FilePath& path,
      FileMode mode = FileMode::Create,
      FileAccess access = FileAccess::ReadWrite);

  void Open(
      const FilePath& path,
      FileMode mode = FileMode::OpenExisting,
      FileAccess access = FileAccess::ReadWrite);

  SystemErrorCode TryCreate(
      const FilePath& path,
      FileMode mode = FileMode::Create,
      FileAccess access = FileAccess::ReadWrite);

  SystemErrorCode TryOpen(
      const FilePath& path,
      FileMode mode = FileMode::OpenExisting,
      FileAccess access = FileAccess::ReadWrite);

  enum NativeFileLifetime : int8_t {
    AutoClose,
    DontClose,
  };
  void OpenNative(
      NativeFile native_file,
      FileAccess access = FileAccess::ReadWrite,
      NativeFileLifetime lifetime = AutoClose);

  void Close() override;
  bool IsOpen() const noexcept override;
  int ReadAtMost(MutableBufferSpan output) override;
  void Write(BufferSpan input) override;
  void PositionalRead(int64_t offset, MutableBufferSpan output) override;
  // Positional write cannot be used with append mode.
  void PositionalWrite(int64_t offset, BufferSpan input) override;
  int64_t Seek(int64_t offset, SeekOrigin origin) override;
  void Flush() override;

  bool CanRead() override;
  bool CanWrite() override;

  // Returns false for sequential files (pipes, TTY, etc.).
  bool CanSeek() override;

  // Truncates the file to the given length. If |length| is greater than the
  // current size of the file, the file is extended with zeros.
  // The file position is unchanged on success and undefined on failure.
  void SetLength(int64_t length) override;
  int64_t GetLength() override;

  void SetPosition(int64_t position) override;
  int64_t GetPosition() override;

  // |creation_time| is not supported on POSIX and must be null.
  void SetTimes(Time last_accessed, Time last_modified, Time creation_time = Time());

  void GetInfo(FileStreamInfo& info);

  // Instructs the filesystem to sync the file to disk.
  // Calling SyncToDisk() does not guarantee file integrity and thus is not a valid
  // substitute for file integrity checks and recovery code-paths for malformed files.
  // It can also be *really* slow, so avoid blocking on SyncToDisk(),
  void SyncToDisk();

  ALWAYS_INLINE NativeFile GetNativeFile() const { return native_.get(); }

  NativeFile ReleaseNativeFile() { return native_.release(); }

 private:
  bool CanSeekInternal();
  void CloseInternal(NativeFile native_file);

  #if OS(WIN)
  win::ScopedHandle native_;
  #elif OS(POSIX)
  posix::FileDescriptor native_;
  #endif
  FileAccess access_ = FileAccess::ReadWrite;
  NativeFileLifetime lifetime_ = AutoClose;
  // Whether file is seekable - lazy initialized.
  // -1=unknown, 0=not seekable, 1=seekable
  int8_t seekable_ = -1;
  #if ASSERT_IS_ON
  bool append_ = false;
  #endif

  FileStream(NativeFile native_file, FileAccess access);

  SystemErrorCode TryOpenInternal(const FilePath& path, FileMode mode, FileAccess access);
};

} // namespace stp

#endif // STP_BASE_IO_FILESTREAM_H_
