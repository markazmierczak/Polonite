// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_SYSTEM_SYSINFO_H_
#define STP_BASE_SYSTEM_SYSINFO_H_

#include "Base/Compiler/Os.h"
#include "Base/FileSystem/FilePath.h"
#include "Base/Time/TimeDelta.h"

namespace stp {

class Version;

enum class OsVersion {
  Unknown,

  #if OS(WIN)
  WinPreVista,
  WinVista,
  Win7,        // Also includes Windows Server 2008 R2.
  Win8,        // Also includes Windows Server 2012.
  Win8_1,      // Also includes Windows Server 2012 R2.
  Win10,       // Also includes Windows 10 Server.
  WinNewer,
  #endif
};

class BASE_EXPORT SysInfo {
 public:
  static int64_t AmountOfPhysicalMemory();
  static int AmountOfPhysicalMemoryMB();

  static int64_t AmountOfAvailablePhysicalMemory();

  // Return the number of (Mega)bytes of virtual memory of this process. A return
  // value of zero means that there is no limit on the available virtual memory.
  static int64_t AmountOfVirtualMemory();
  static int AmountOfVirtualMemoryMB();

  // Returns the duration since the startup of host machine.
  static TimeDelta Uptime();

  static String OsName();

  // Not all members are guaranteed to be valid (only major and minor).
  static Version OsVersionNumbers();

  static OsVersion GetOsVersion();

  enum class CpuArch {
    Unknown,
    Intel, // x86
    Amd64, // x64
    Arm,
    Arm64, // AArch64
    Mips,
    PowerPc,
  };
  // Returns the architecture of the running operating system.
  static CpuArch OsArch();

  // Returns the smallest amount of memory (in bytes) which the VM system will allocate.
  static int VmAllocationGranularity();

 private:
  static OsVersion GetOsVersionNative();
};

inline int SysInfo::AmountOfPhysicalMemoryMB() {
  return static_cast<int>(AmountOfPhysicalMemory() >> 20);
}

inline int SysInfo::AmountOfVirtualMemoryMB() {
  return static_cast<int>(AmountOfVirtualMemory() >> 20);
}

} // namespace stp

#endif // STP_BASE_SYSTEM_SYSINFO_H_
