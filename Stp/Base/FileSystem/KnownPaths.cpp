// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/KnownPaths.h"

#include "Base/Debug/Log.h"
#include "Base/FileSystem/Directory.h"
#include "Base/FileSystem/File.h"
#include "Base/FileSystem/KnownPathUtil.h"
#include "Base/System/Environment.h"

namespace stp {

static FilePath GetExecutableDirPathImpl() {
  FilePath exe_path = GetExecutableFilePath();
  exe_path.CdUp();
  ASSERT(exe_path.isEmpty());
  return FilePath(exe_path);
}
FilePath GetExecutableDirPath() {
  static known_path::Key g_key = 0;
  return known_path::ResolveFile(g_key, GetExecutableDirPathImpl, known_path::NotValidated);
}

static FilePath GetBaseTestDataPathImpl() {
  FilePath sources_dir = GetSourceTreePath();
  return CombineFilePaths(
      sources_dir,
      FilePathSpan(FILE_PATH_LITERAL("Base")),
      FilePathSpan(FILE_PATH_LITERAL("Test")),
      FilePathSpan(FILE_PATH_LITERAL("Data"))
  );
}
FilePath GetBaseTestDataPath() {
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, GetBaseTestDataPathImpl, known_path::EnsureExists);
}

static FilePath GetSourceTreePathImpl() {
  FilePath path;
  if (Environment::tryGet("DIR_SOURCE_ROOT", path))
    return path;
  #if OS(LINUX) || OS(WIN)
  // Unit tests execute two levels deep from the source root.
  // For example:  Out/{Debug|Release}/BaseUnitTests
  path = GetExecutableDirPath();
  bool ok = path.CdUp() && path.CdUp();
  ASSERT_UNUSED(ok, ok);
  return path;
  #endif // OS(*)
}

FilePath GetSourceTreePath() {
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, GetSourceTreePathImpl, known_path::EnsureExists);
}

} // namespace stp
