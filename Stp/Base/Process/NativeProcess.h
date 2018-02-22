// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_PROCESS_NATIVEPROCESS_H_
#define STP_BASE_PROCESS_NATIVEPROCESS_H_

#include "Base/Compiler/Os.h"
#include "Base/FileSystem/FilePath.h"

#include <sys/types.h>

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#endif

namespace stp {

// NativeProcessHandle is a platform specific type which represents the underlying OS
// handle to a process.
// NativeProcessId is a number which identifies the process in the OS.
#if OS(WIN)
typedef HANDLE NativeProcessHandle;
typedef DWORD NativeProcessId;
const NativeProcessHandle NullNativeProcessHandle = NULL;
const NativeProcessId NullNativeProcessId = 0;
#elif OS(POSIX)
// On POSIX, our NativeProcessHandle will just be the PID.
typedef pid_t NativeProcessHandle;
typedef pid_t NativeProcessId;
const NativeProcessHandle NullNativeProcessHandle = 0;
const NativeProcessId NullNativeProcessId = 0;
#endif // OS(*)

class BASE_EXPORT NativeProcess {
 public:
  // Returns the id of the current process.
  // Note that on some platforms, this is not guaranteed to be unique across processes.
  static NativeProcessId GetCurrentId();

  // Returns the NativeProcessHandle of the current process.
  static NativeProcessHandle getCurrentHandle();

  // Returns the ID for the parent of the given process.
  static NativeProcessId GetParentId(NativeProcessHandle process);

  #if OS(LINUX) || OS(ANDROID)
  // Returns the path to the executable of the given process.
  static FilePath getExecutablePath(NativeProcessHandle process);
  #endif
};

} // namespace stp

#endif // STP_BASE_PROCESS_NATIVEPROCESS_H_
