// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/System/SysInfo.h"

#include "Base/Debug/Log.h"
#include "Base/Type/Limits.h"
#include "Base/Util/Version.h"

#include <windows.h>

namespace stp {

static int64_t AmountOfMemory(DWORDLONG MEMORYSTATUSEX::*memory_field) {
  MEMORYSTATUSEX memory_info;
  memory_info.dwLength = sizeof(memory_info);
  if (!GlobalMemoryStatusEx(&memory_info)) {
    ASSERT(false);
    return 0;
  }
  int64_t rv = static_cast<int64_t>(memory_info.*memory_field);
  return rv < 0 ? Limits<int64_t>::Max : rv;
}

int64_t SysInfo::AmountOfPhysicalMemory() {
  return AmountOfMemory(&MEMORYSTATUSEX::ullTotalPhys);
}

int64_t SysInfo::AmountOfAvailablePhysicalMemory() {
  return AmountOfMemory(&MEMORYSTATUSEX::ullAvailPhys);
}

int64_t SysInfo::AmountOfVirtualMemory() {
  return AmountOfMemory(&MEMORYSTATUSEX::ullTotalVirtual);
}

String SysInfo::OsName() {
  return "Windows NT";
}

Version SysInfo::OsVersionNumbers() {
  OSVERSIONINFOEX info = { sizeof info };
  ::GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&info));
  return Version(info.dwMajorVersion, info.dwMinorVersion, 0, info.dwBuildNumber);
}

OsVersion SysInfo::GetOsVersionNative() {
  OSVERSIONINFOEX version_info = { sizeof version_info };
  ::GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&version_info));
  DWORD major = version_info.dwMajorVersion;
  DWORD minor = version_info.dwMinorVersion;

  if (major <= 5) {
    // Treat XP Pro x64, Home Server, and Server 2003 R2 as Server 2003.
    return OsVersion::WinPreVista;
  }
  if (major == 6) {
    switch (minor) {
      case 0:
        // Treat Windows Server 2008 the same as Windows Vista.
        return OsVersion::WinVista;
      case 1:
        // Treat Windows Server 2008 R2 the same as Windows 7.
        return OsVersion::Win7;
      case 2:
        // Treat Windows Server 2012 the same as Windows 8.
        return OsVersion::Win8;
      default:
        ASSERT(minor == 3);
        return OsVersion::Win8_1;
    }
  }
  if (major == 10)
    return OsVersion::Win10;

  ASSERT(major > 10);
  LOG(ERROR, "unknown Windows version");
  return OsVersion::WinNewer;
}

SysInfo::CpuArch SysInfo::OsArch() {
  SYSTEM_INFO system_info = {};
  ::GetNativeSystemInfo(&system_info);
  switch (system_info.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_INTEL:
      return CpuArch::Intel;
    case PROCESSOR_ARCHITECTURE_AMD64:
      return CpuArch::Amd64;
    case PROCESSOR_ARCHITECTURE_ARM:
      return CpuArch::Arm;
    case PROCESSOR_ARCHITECTURE_ARM64:
      return CpuArch::Arm64;
  }
  ASSERT(false, "unknown architecture");
  return CpuArch::Unknown;
}

int SysInfo::VmAllocationGranularity() {
  SYSTEM_INFO system_info = {};
  ::GetNativeSystemInfo(&system_info);
  return system_info.dwAllocationGranularity;
}

} // namespace stp
