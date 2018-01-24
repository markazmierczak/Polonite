// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Linux/ProcCommon.h"

#include "Base/FileSystem/FilePathWriter.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Type/Formattable.h"
#include "Base/Type/ParseInteger.h"

#include <limits.h>

namespace stp {
namespace linux {

FilePath ProcCommon::GetRootDirectory() {
  return FilePath(FILE_PATH_LITERAL("/proc"));
}

FilePath ProcCommon::DirectoryForProcess(NativeProcessHandle pid) {
  if (pid == NativeProcess::GetCurrentHandle())
    return FilePath(FILE_PATH_LITERAL("/proc/self"));

  FilePath path = GetRootDirectory();
  FilePathWriter writer(path);
  writer.EnsureSeparator();
  writer << pid;
  return path;
}

NativeProcessHandle ProcCommon::ProcessForDirectoryName(const char* d_name) {
  int i;
  for (i = 0; i < NAME_MAX && d_name[i]; ++i) {
    if (!IsDigitAscii(d_name[i]))
      return 0;
  }
  if (i == NAME_MAX)
    return 0;

  return ParseTo<pid_t>(StringSpan(d_name, i));
}

} // namespace linux
} // namespace stp
