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

  void create(
      const FilePath& path,
      FileMode mode = FileMode::Create,
      FileAccess access = FileAccess::ReadWrite);

  void open(
      const FilePath& path,
      FileMode mode = FileMode::OpenExisting,
      FileAccess access = FileAccess::ReadWrite);

  SystemErrorCode tryCreate(
      const FilePath& path,
      FileMode mode = FileMode::Create,
      FileAccess access = FileAccess::ReadWrite);

  SystemErrorCode tryOpen(
      const FilePath& path,
      FileMode mode = FileMode::OpenExisting,
      FileAccess access = FileAccess::ReadWrite);

  enum NativeFileLifetime : int8_t {
    AutoClose,
    DontClose,
  };
  void openNative(
      NativeFile native_file,
      FileAccess access = FileAccess::ReadWrite,
      NativeFileLifetime lifetime = AutoClose);

  void close() override;
  bool isOpen() const override;
  int readAtMost(MutableBufferSpan output) override;
  void write(BufferSpan input) override;
  void positionalRead(int64_t offset, MutableBufferSpan output) override;
  // Positional write cannot be used with append mode.
  void positionalWrite(int64_t offset, BufferSpan input) override;
  int64_t seek(int64_t offset, SeekOrigin origin) override;
  void flush() override;

  bool canRead() override;
  bool canWrite() override;

  // Returns false for sequential files (pipes, TTY, etc.).
  bool canSeek() override;

  // Truncates the file to the given length. If |length| is greater than the
  // current size of the file, the file is extended with zeros.
  // The file position is unchanged on success and undefined on failure.
  void setLength(int64_t length) override;
  int64_t getLength() override;

  void setPosition(int64_t position) override;
  int64_t getPosition() override;

  // |creation_time| is not supported on POSIX and must be null.
  void setTimes(Time last_accessed, Time last_modified, Time creation_time = Time());

  void getInfo(FileStreamInfo& info);

  // Instructs the filesystem to sync the file to disk.
  // Calling SyncToDisk() does not guarantee file integrity and thus is not a valid
  // substitute for file integrity checks and recovery code-paths for malformed files.
  // It can also be *really* slow, so avoid blocking on SyncToDisk(),
  void syncToDisk();

  ALWAYS_INLINE NativeFile getNativeFile() const { return native_.get(); }

  NativeFile releaseNativeFile() { return native_.leakDescriptor(); }

 private:
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

  SystemErrorCode tryOpenInternal(const FilePath& path, FileMode mode, FileAccess access);

  bool canSeekInternal();
  void closeInternal(NativeFile native_file);
};

} // namespace stp

#endif // STP_BASE_IO_FILESTREAM_H_
