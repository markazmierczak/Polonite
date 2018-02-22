// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/KnownPaths.h"

#include "Base/FileSystem/KnownPathUtil.h"
#include "Base/System/Environment.h"
#include "Base/Error/SystemException.h"

#include <unistd.h>

namespace stp {

FilePath getTempDirPath() {
  auto provider = []() {
    FilePath path;
    if (Environment::tryGet("TMPDIR", path))
      return path;

    #if OS(ANDROID)
    path = getAppCachePath();
    #else
    path = FILE_PATH_LITERAL("/tmp");
    #endif
    return path;
  };
  static known_path::Key g_key = 0;
  return known_path::resolveDirectory(g_key, provider, known_path::NotValidated);
}

FilePath getHomeDirPath() {
  auto provider = []() {
    #if OS(ANDROID)
    throw Exception::with(
        NotSupportedException(),
        "home directory lookup not yet implemented for Android");
    #else
    FilePath path;
    if (Environment::tryGet("HOME", path))
      return path;

    ASSERT(false, "unable to get home directory");
    #endif
    return getTempDirPath();
  };
  static known_path::Key g_key = 0;
  return known_path::resolveDirectory(g_key, provider, known_path::NotValidated);
}

FilePath getCurrentDirPath() {
  // Cannot be cached through "resolver" mechanism for obvious reason
  // (current directory is not a constant).
  constexpr int StackLength = 256;
  char stack_buffer[StackLength];
  if (::getcwd(stack_buffer, StackLength) != nullptr)
    return FilePath(makeFilePathSpanFromNullTerminated(stack_buffer));

  FilePath path;
  for (int buffer_len = StackLength * 2; errno == ERANGE; buffer_len *= 2) {
    path.clear();
    char* dst = path.chars().appendUninitialized(buffer_len);
    if (::getcwd(dst, buffer_len) != nullptr) {
      int real_length = makeSpanFromNullTerminated(dst).size();
      path.truncate(real_length);
      return path;
    }
  }
  throw SystemException(getLastSystemErrorCode());
}

bool setCurrentDirPath(const FilePath& path) {
  return ::chdir(toNullTerminated(path)) == 0;
}

} // namespace stp
