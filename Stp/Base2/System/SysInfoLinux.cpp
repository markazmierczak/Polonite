// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/SysInfo.h"

#include "Base/Util/Version.h"

#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace stp {

static int64_t AmountOfMemoryHelper(int pages_name) {
  long pages = sysconf(pages_name);
  long page_size = sysconf(_SC_PAGESIZE);
  if (pages == -1 || page_size == -1) {
    ASSERT(false, "sysconf failed");
    return 0;
  }
  return static_cast<int64_t>(pages) * page_size;
}

int64_t SysInfo::AmountOfAvailablePhysicalMemory() {
  return AmountOfMemoryHelper(_SC_AVPHYS_PAGES);
}

int64_t SysInfo::AmountOfPhysicalMemory() {
  return AmountOfMemoryHelper(_SC_PHYS_PAGES);
}

#if !OS(ANDROID)
Version SysInfo::OsVersionNumbers() {
  struct utsname info;
  if (uname(&info) < 0) {
    ASSERT(false);
    return Version();
  }
  int major;
  int minor;
  int bugfix;
  int num_read = sscanf(info.release, "%d.%d.%d", &major, &minor, &bugfix);
  if (num_read < 1)
    major = 0;
  if (num_read < 2)
    minor = 0;
  if (num_read < 3)
    bugfix = 0;

  return Version(major, minor, bugfix);
}
#endif // OS(*)

} // namespace stp
