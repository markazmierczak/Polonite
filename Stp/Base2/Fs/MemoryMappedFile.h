// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_FS_MEMORYMAPPEDFILE_H_
#define STP_BASE_FS_MEMORYMAPPEDFILE_H_

#include "Base/Io/FileStream.h"

namespace stp {

class FilePath;

class BASE_EXPORT MemoryMappedFile {
 public:
  enum Access {
    // Mapping a file into memory effectively allows for file I/O on any thread.
    // The accessing thread could be paused while data from the file is paged
    // into memory. Worse, a corrupted filesystem could cause a SEGV within the
    // program instead of just an I/O error.
    ReadOnly,

    // This provides read/write access to a file and must be used with care of
    // the additional subtleties involved in doing so. Though the OS will do
    // the writing of data on its own time, too many dirty pages can cause
    // the OS to pause the thread while it writes them out. The pause can
    // be as much as 1s on some systems.
    ReadWrite,

    // This provides read/write access but with the ability to write beyond
    // the end of the existing file up to a maximum size specified as the
    // "region". Depending on the OS, the file may or may not be immediately
    // extended to the maximum size though it won't be loaded in RAM until
    // needed. Note, however, that the maximum size will still be reserved
    // in the process address space.
    ReadWriteExtend,
  };

  // The default constructor sets all members to invalid/null values.
  MemoryMappedFile();
  ~MemoryMappedFile();

  // Used to hold information about a region [offset + size] of a file.
  struct BASE_EXPORT Region {
    static const Region WholeFile;

    bool operator ==(const Region& other) const {
      return other.offset == offset && other.size == size;
    }
    bool operator!=(const Region& other) const {
      return !operator==(other);
    }

    // Start of the region (measured in bytes from the beginning of the file).
    int64_t offset;

    // Length of the region in bytes.
    int size;
  };

  // Opens an existing file and maps it into memory. |access| can be read-only
  // or read/write but not read/write+extend. If this object already points
  // to a valid memory mapped file then this method will fail and return
  // false. If it cannot open the file, the file does not exist, or the
  // memory mapping fails, it will return false.
  bool Initialize(const FilePath& file_name, Access access = ReadOnly);

  // As above, but works with an already-opened file. |access| can be read-only
  // or read/write but not read/write+extend. MemoryMappedFile takes ownership
  // of |file| and closes it when done. |file| must have been opened with
  // permissions suitable for |access|. If the memory mapping fails, it will
  // return false.
  bool Initialize(FileStream&& file, Access access = ReadOnly);

  // As above, but works with a region of an already-opened file. All forms of
  // |access| are allowed. If ReadWriteExtend is specified then |region|
  // provides the maximum size of the file. If the memory mapping fails, it
  // return false.
  bool Initialize(FileStream&& file, const Region& region, Access access = ReadOnly);

  ALWAYS_INLINE const char* data() const { return data_; }
  ALWAYS_INLINE char* data() { return data_; }
  ALWAYS_INLINE int length() const { return length_; }

  // Is file_ a valid file handle that points to an open, memory mapped file?
  ALWAYS_INLINE bool IsValid() const { return data_ != nullptr; }

 private:
  // Given the arbitrarily aligned memory region [start, size], returns the
  // boundaries of the region aligned to the granularity specified by the OS,
  // (a page on Linux, ~32k on Windows) as follows:
  // - |aligned_start| is page aligned and <= |start|.
  // - |aligned_size| is a multiple of the VM granularity and >= |size|.
  // - |offset| is the displacement of |start| w.r.t |aligned_start|.
  struct VmAlignedBoundaries {
    int64_t start;
    int size;
    int offset;
  };
  static VmAlignedBoundaries ComputeVmAlignedBoundaries(int64_t start, int size);

  // Map the file to memory, set data_ to that memory address. Return true on
  // success, false on any kind of failure. This is a helper for Initialize().
  bool MapFileRegionToMemory(const Region& region, Access access);

  // Closes all open handles.
  void CloseHandles();

  FileStream file_;
  char* data_ = nullptr;
  int length_ = 0;

  #if OS(WIN)
  win::ScopedHandle file_mapping_;
  #endif

  DISALLOW_COPY_AND_ASSIGN(MemoryMappedFile);
};

} // namespace stp

#endif // STP_BASE_FS_MEMORYMAPPEDFILE_H_
