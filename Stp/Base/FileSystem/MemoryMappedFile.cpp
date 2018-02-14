// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/FileSystem/MemoryMappedFile.h"

#include "Base/Debug/Log.h"
#include "Base/FileSystem/FilePath.h"
#include "Base/System/SysInfo.h"
#include "Base/Text/FormatMany.h"

namespace stp {

const MemoryMappedFile::Region MemoryMappedFile::Region::WholeFile = {0, -1};

MemoryMappedFile::MemoryMappedFile() {
}

MemoryMappedFile::~MemoryMappedFile() {
  CloseHandles();
}

bool MemoryMappedFile::Initialize(const FilePath& file_name, Access access) {
  if (IsValid())
    return false;

  FileMode file_mode = FileMode::OpenExisting;
  FileAccess file_access;
  switch (access) {
    case ReadOnly:
      file_access = FileAccess::ReadOnly;
      break;
    case ReadWrite:
      file_access = FileAccess::ReadWrite;
      break;
    default:
      // Can't open with "extend" because no maximum size is known.
      file_access = FileAccess::ReadOnly;
      ASSERT(false);
  }
  if (!file_.TryOpen(file_name, file_mode, file_access)) {
    LOG(ERROR, "couldn't open {}", file_name);
    return false;
  }

  if (!MapFileRegionToMemory(Region::WholeFile, access)) {
    CloseHandles();
    return false;
  }
  return true;
}

bool MemoryMappedFile::Initialize(FileStream&& file, Access access) {
  ASSERT(access != ReadWriteExtend);
  return Initialize(Move(file), Region::WholeFile, access);
}

bool MemoryMappedFile::Initialize(FileStream&& file, const Region& region, Access access) {
  switch (access) {
    case ReadWriteExtend:
      // Ensure that the extended size is within limits of File.
      if (region.size > Limits<int64_t>::Max - region.offset) {
        LOG(ERROR, "region bounds exceed maximum for File");
        return false;
      }
      // no break
    case ReadOnly:
    case ReadWrite:
      // Ensure that the region values are valid.
      if (region.offset < 0 || region.size < 0) {
        LOG(ERROR, "region bounds are not valid");
        return false;
      }
      break;
  }

  if (IsValid())
    return false;

  if (region != Region::WholeFile) {
    ASSERT(region.offset >= 0);
    ASSERT(region.size > 0);
  }

  file_ = Move(file);

  if (!MapFileRegionToMemory(region, access)) {
    CloseHandles();
    return false;
  }
  return true;
}

MemoryMappedFile::VmAlignedBoundaries MemoryMappedFile::ComputeVmAlignedBoundaries(
    int64_t start, int size) {
  // Sadly, on Windows, the mmap alignment is not just equal to the page size.
  const int64_t mask = static_cast<int64_t>(SysInfo::VmAllocationGranularity()) - 1;
  ASSERT(mask < INT32_MAX);

  VmAlignedBoundaries rv;
  rv.offset = start & mask;
  rv.start = start & ~mask;
  rv.size = (size + rv.offset + mask) & ~mask;
  return rv;
}

} // namespace stp
