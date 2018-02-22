// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/KnownPaths.h"

#include "Base/Debug/Log.h"
#include "Base/FileSystem/Directory.h"
#include "Base/FileSystem/File.h"
#include "Base/FileSystem/KnownPathUtil.h"
#include "Base/System/Environment.h"

namespace stp {

static FilePath getExecutableDirPathImpl() {
  FilePath exe_path = getExecutableFilePath();
  exe_path.cdUp();
  ASSERT(exe_path.isEmpty());
  return FilePath(exe_path);
}
FilePath getExecutableDirPath() {
  static known_path::Key g_key = 0;
  return known_path::resolveFile(g_key, getExecutableDirPathImpl, known_path::NotValidated);
}

static FilePath getBaseTestDataPathImpl() {
  FilePath sources_dir = getSourceTreePath();
  return combineFilePaths(
      sources_dir,
      FilePathSpan(FILE_PATH_LITERAL("Base")),
      FilePathSpan(FILE_PATH_LITERAL("Test")),
      FilePathSpan(FILE_PATH_LITERAL("Data"))
  );
}
FilePath getBaseTestDataPath() {
  static known_path::Key g_key = 0;
  return known_path::resolveDirectory(g_key, getBaseTestDataPathImpl, known_path::EnsureExists);
}

static FilePath getSourceTreePathImpl() {
  FilePath path;
  if (Environment::tryGet("DIR_SOURCE_ROOT", path))
    return path;
  #if OS(LINUX) || OS(WIN)
  // Unit tests execute two levels deep from the source root.
  // For example:  Out/{Debug|Release}/BaseUnitTests
  path = getExecutableDirPath();
  bool ok = path.cdUp() && path.cdUp();
  ASSERT_UNUSED(ok, ok);
  return path;
  #endif // OS(*)
}

FilePath getSourceTreePath() {
  static known_path::Key g_key = 0;
  return known_path::resolveDirectory(g_key, getSourceTreePathImpl, known_path::EnsureExists);
}

} // namespace stp
