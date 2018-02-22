// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/KnownPaths.h"

#include "Base/FileSystem/FilePath.h"
#include "Base/Linux/ProcCommon.h"

#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>

namespace stp {

FilePath getExecutableFilePath() {
  return NativeProcess::getExecutablePath(NativeProcess::getCurrentHandle());
}

} // namespace stp
