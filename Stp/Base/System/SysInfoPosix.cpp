// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/SysInfo.h"

#include "Base/Type/Limits.h"

#include <errno.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace stp {

int64_t SysInfo::AmountOfVirtualMemory() {
  struct rlimit limit;
  int result = getrlimit(RLIMIT_DATA, &limit);
  if (result != 0) {
    ASSERT(false);
    return 0;
  }
  return limit.rlim_cur == RLIM_INFINITY ? 0 : limit.rlim_cur;
}

#if OS(LINUX) || OS(BSD)
String SysInfo::OsName() {
  struct utsname info;
  if (uname(&info) < 0) {
    ASSERT(false);
    return String();
  }
  return String(makeSpanFromNullTerminated(info.sysname));
}
#endif

SysInfo::CpuArch SysInfo::OsArch() {
  struct utsname info;
  if (uname(&info) < 0) {
    ASSERT(false);
    return CpuArch::Unknown;
  }
  auto arch = makeSpanFromNullTerminated(info.machine);
  if (arch == "i386" || arch == "i486" || arch == "i586" || arch == "i686")
    return CpuArch::Intel;
  if (arch == "amd64")
    return CpuArch::Amd64;

  ASSERT(false, "unknown architecture");
  return CpuArch::Unknown;
}

int SysInfo::VmAllocationGranularity() {
  return getpagesize();
}

} // namespace stp
