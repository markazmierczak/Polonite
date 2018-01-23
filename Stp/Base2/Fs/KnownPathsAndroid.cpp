// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Fs/KnownPaths.h"

#include "Base/Fs/FilePath.h"
#include "Base/Linux/ProcCommon.h"

#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>

namespace stp {

FilePath GetExecutableFilePath() {
  return NativeProcess::GetExecutablePath(NativeProcess::GetCurrentHandle());
}

} // namespace stp
