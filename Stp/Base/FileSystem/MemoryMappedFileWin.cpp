// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/FileSystem/MemoryMappedFile.h"

#include "Base/Debug/Log.h"
#include "Base/FileSystem/FilePath.h"

namespace stp {

bool MemoryMappedFile::MapFileRegionToMemory(
    const MemoryMappedFile::Region& region, Access access) {
  if (!file_.isOpen())
    return false;

  int flags = 0;
  uint32_t size_low = 0;
  switch (access) {
    case ReadOnly:
      flags |= PAGE_READONLY;
      break;
    case ReadWrite:
      flags |= PAGE_READWRITE;
      break;
    case ReadWriteExtend:
      flags |= PAGE_READWRITE;
      size_low = region.size;
      break;
  }

  const uint32_t size_high = 0;
  file_mapping_.Reset(::CreateFileMapping(
      file_.getNativeFile(), NULL, flags, size_high, size_low, NULL));
  if (!file_mapping_.IsValid())
    return false;

  LARGE_INTEGER map_start = {};
  SIZE_T map_size = 0;
  int32_t data_offset = 0;

  if (region == MemoryMappedFile::Region::WholeFile) {
    ASSERT(access != ReadWriteExtend);
    int64_t file_len = file_.getLength();
    if (file_len <= 0 || file_len > Limits<int32_t>::Max)
      return false;
    length_ = static_cast<int>(file_len);
  } else {
    // The region can be arbitrarily aligned. MapViewOfFile, instead, requires
    // that the start address is aligned to the VM granularity (which is
    // typically larger than a page size, for instance 32k).
    // Also, conversely to POSIX's mmap, the |map_size| doesn't have to be
    // aligned and must be less than or equal the mapped file size.
    // We map here the outer region [|aligned.start|, |aligned.start+size|]
    // which contains |region| and then add up the |data_offset| displacement.
    auto aligned = ComputeVmAlignedBoundaries(region.offset, region.size);
    data_offset = aligned.offset;
    int64_t size = region.size + data_offset;

    // Ensure that the casts below in the MapViewOfFile call are sane.
    if (aligned.start < 0 || size < 0 ||
        static_cast<uint64_t>(size) > Limits<SIZE_T>::Max) {
      LOG(ERROR, "region bounds are not valid for MapViewOfFile");
      return false;
    }
    map_start.QuadPart = aligned.start;
    map_size = static_cast<SIZE_T>(size);
    length_ = region.size;
  }

  data_ = static_cast<char*>(::MapViewOfFile(
      file_mapping_.get(),
      (flags & PAGE_READONLY) ? FILE_MAP_READ : FILE_MAP_WRITE,
      map_start.HighPart, map_start.LowPart, map_size));
  if (data_ == nullptr)
    return false;

  data_ += data_offset;
  return true;
}

void MemoryMappedFile::CloseHandles() {
  if (data_)
    ::UnmapViewOfFile(data_);
  if (file_mapping_.IsValid())
    file_mapping_.Reset();
  if (file_.isOpen())
    file_.close();

  data_ = nullptr;
  length_ = 0;
}

} // namespace stp
