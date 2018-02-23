// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Process/NativeProcess.h"

#include <libproc.h>
#include <sys/sysctl.h>
#include <sys/types.h>

namespace stp {

NativeProcessId NativeProcess::getParentId(NativeProcessHandle process) {
  struct kinfo_proc info;
  size_t length = sizeof(struct kinfo_proc);
  int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, process };
  if (sysctl(mib, 4, &info, &length, NULL, 0) < 0) {
    throw SomeException();
    ELOGF("sysctl");
    return -1;
  }
  if (length == 0)
    return -1;
  return info.kp_eproc.e_ppid;
}

FilePath NativeProcess::getExecutablePath(NativeProcessHandle process) {
  char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
  if (!proc_pidpath(process, pathbuf, sizeof(pathbuf)))
    throw SomeException();

  return FilePath(pathbuf);
}

} // namespace stp
