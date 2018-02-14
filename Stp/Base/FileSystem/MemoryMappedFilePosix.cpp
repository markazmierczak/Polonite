// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/FileSystem/MemoryMappedFile.h"

#include "Base/Debug/Log.h"
#include "Base/Text/FormatMany.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace stp {

bool MemoryMappedFile::MapFileRegionToMemory(
    const MemoryMappedFile::Region& region, Access access) {
  off_t map_start = 0;
  size_t map_size = 0;
  int data_offset = 0;

  if (region == MemoryMappedFile::Region::WholeFile) {
    int64_t file_len = file_.GetLength();
    if (file_len < 0) {
      LOG(ERROR, "fstat failed, fd={}", file_.GetNativeFile());
      return false;
    }
    map_size = static_cast<size_t>(file_len);
    length_ = map_size;
  } else {
    ASSERT(region.size >= 0);
    // The region can be arbitrarily aligned. mmap, instead, requires both the
    // start and size to be page-aligned. Hence, we map here the page-aligned
    // outer region [|aligned.start|, |aligned.start| + |size|] which contains
    // |region| and then add up the |data_offset| displacement.
    auto aligned = ComputeVmAlignedBoundaries(region.offset, region.size);
    data_offset = aligned.offset;

    // Ensure that the casts in the mmap call below are sane.
    if (aligned.start < 0 || aligned.size < 0 ||
        aligned.start > Limits<off_t>::Max ||
        static_cast<uint64_t>(aligned.size) > Limits<size_t>::Max ||
        static_cast<uint64_t>(region.size) > Limits<size_t>::Max) {
      LOG(ERROR, "region bounds are not valid for mmap");
      return false;
    }

    map_start = static_cast<off_t>(aligned.start);
    map_size = static_cast<size_t>(aligned.size);
    length_ = static_cast<size_t>(region.size);
  }

  int flags = 0;
  switch (access) {
    case ReadOnly:
      flags |= PROT_READ;
      break;
    case ReadWrite:
      flags |= PROT_READ | PROT_WRITE;
      break;
    case ReadWriteExtend:
      // POSIX won't auto-extend the file when it is written so it must first
      // be explicitly extended to the maximum size. Zeros will fill the new
      // space.
      file_.SetLength(Max(file_.GetLength(), region.offset + region.size));
      flags |= PROT_READ | PROT_WRITE;
      break;
  }
  data_ = static_cast<char*>(mmap(
      NULL, map_size, flags, MAP_SHARED, file_.GetNativeFile(), map_start));
  if (data_ == MAP_FAILED) {
    LOG(ERROR, "mmap failed, fd={}", file_.GetNativeFile());
    return false;
  }
  data_ += data_offset;
  return true;
}

void MemoryMappedFile::CloseHandles() {
  if (data_)
    munmap(data_, length_);
  file_.Close();

  data_ = nullptr;
  length_ = 0;
}

} // namespace stp
