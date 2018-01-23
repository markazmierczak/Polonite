// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/SysInfo.h"

#include <mach/mach_host.h>
#include <mach/mach_init.h>

int64_t SysInfo::AmountOfPhysicalMemory() {
  struct host_basic_info hostinfo;
  mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
  darwin::ScopedMachSendRight host(mach_host_self());
  int result = host_info(
      host.get(), HOST_BASIC_INFO,
      reinterpret_cast<host_info_t>(&hostinfo), &count);
  if (result != KERN_SUCCESS) {
    ASSERT(false);
    return 0;
  }
  ASSERT(count == HOST_BASIC_INFO_COUNT);
  return static_cast<int64_t>(hostinfo.max_mem);
}

int64_t SysInfo::AmountOfAvailablePhysicalMemory() {
  darwin::ScopedMachSendRight host(mach_host_self());
  vm_statistics_data_t vm_info;
  mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
  int result = host_statistics(
      host.get(), HOST_VM_INFO,
      reinterpret_cast<host_info_t>(&vm_info), &count);
  if (result != KERN_SUCCESS) {
    ASSERT(false);
    return 0;
  }
  auto available_pages = vm_info.free_count - vm_info.speculative_count;
  return static_cast<int64_t>(available_pages) * PAGE_SIZE;
}

Version SysInfo::OsVersionNumbers() {
  auto osv = NSProcessInfo.processInfo.operatingSystemVersion;
  return Version(osv.majorVersion, osv.minorVersion, osv.patchVersion);
}

} // namespace stp
