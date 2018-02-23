// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Process/NativeProcess.h"

#include "Base/Win/ScopedHandle.h"

#include <windows.h>
#include <tlhelp32.h>

namespace stp {

NativeProcessId NativeProcess::getCurrentId() {
  return ::GetCurrentProcessId();
}

NativeProcessHandle NativeProcess::getCurrentHandle() {
  return ::GetCurrentProcess();
}

NativeProcessId NativeProcess::getParentId(NativeProcessHandle process) {
  NativeProcessId child_pid = ::GetProcessId(process);
  PROCESSENTRY32 process_entry;
  process_entry.dwSize = sizeof(PROCESSENTRY32);

  win::ScopedHandle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
  if (snapshot.IsValid() && Process32First(snapshot.get(), &process_entry)) {
    do {
      if (process_entry.th32ProcessID == child_pid)
        return process_entry.th32ParentProcessID;
    } while (Process32Next(snapshot.get(), &process_entry));
  }
  return 0;
}

} // namespace stp
