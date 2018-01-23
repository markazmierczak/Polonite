// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/SysInfo.h"

#include "Base/Compiler/Os.h"
#include "Base/Time/TimeTicks.h"

namespace stp {

// This code relies on an implementation detail of TimeTicks::Now() - that
// its return value happens to coincide with the system uptime value in
// microseconds, on Win/Mac/iOS/Linux and Android.
#if OS(WIN) || OS(DARWIN) || OS(LINUX) || OS(ANDROID)
TimeDelta SysInfo::Uptime() {
  int64_t uptime_in_microseconds = TimeTicks::Now().ToInternalValue();
  return TimeDelta::FromMicroseconds(uptime_in_microseconds);
}
#endif

#if OS(MAC) || OS(ANDROID)
String SysInfo::OsName() {
  #if OS(MAC)
  return "Mac OS X";
  #elif OS(ANDROID)
  return "Android";
  #endif
}
#endif

OsVersion SysInfo::GetOsVersion() {
  static OsVersion g_version = OsVersion::Unknown;
  if (g_version == OsVersion::Unknown)
    g_version = GetOsVersionNative();
  return g_version;
}

#if OS(LINUX)
OsVersion SysInfo::GetOsVersionNative() {
  return OsVersion::Unknown;
}
#endif

} // namespace stp
