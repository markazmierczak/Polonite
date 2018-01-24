// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Process/NativeProcess.h"

#include "Base/FileSystem/File.h"
#include "Base/Linux/ProcCommon.h"

namespace stp {

NativeProcessId NativeProcess::GetParentId(NativeProcessHandle process) {
  return getppid();
}

FilePath NativeProcess::GetExecutablePath(NativeProcessHandle process) {
  FilePath exe_file = linux::ProcCommon::DirectoryForProcess(process);
  exe_file.AddAscii("exe");
  return File::ReadSymbolicLink(exe_file);
}

} // namespace stp
