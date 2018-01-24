// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_LINUX_PROCCOMMON_H_
#define STP_BASE_LINUX_PROCCOMMON_H_

#include "Base/FileSystem/FilePath.h"
#include "Base/Process/NativeProcess.h"

namespace stp {
namespace linux {

class BASE_EXPORT ProcCommon {
 public:
  static FilePath GetRootDirectory();

  // Returns a FilePath to "/proc/<pid>".
  static FilePath DirectoryForProcess(NativeProcessHandle pid);

  // Take a /proc directory entry named |d_name|, and if it is the directory for
  // a process, convert it to a pid_t.
  // Returns 0 on failure.
  // e.g. /proc/self/ will return 0, whereas /proc/1234 will return 1234.
  static NativeProcessHandle ProcessForDirectoryName(const char* d_name);
};

} // namespace linux
} // namespace stp

#endif // STP_BASE_LINUX_PROCCOMMON_H_
