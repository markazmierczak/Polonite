// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/KnownPaths.h"

#import <Foundation/Foundation.h>
#include <mach-o/dyld.h>

namespace stp {

static FilePath appendAppName(FilePath path) {
  path.appendAscii(Application::Instance().GetShortName());
  return path;
}

FilePath GetTempDirPath() {
  auto provider = []() {
    NSString* tmp = NSTemporaryDirectory();
    ASSERT(tmp != nil);
    return darwin::NSStringToFilePath(tmp);
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::NotValidated);
}

FilePath GetHomeDirPath() {
  auto provider = []() {
    NSString* tmp = NSHomeDirectory();
    ASSERT(tmp != nil);
    return darwin::NSStringToFilePath(tmp);
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::NotValidated);
}

FilePath GetExecutableFilePath() {
  auto provider = []() {
    // Executable path can have relative references ("..") depending on
    // how the app was launched.
    uint32_t executable_length = 0;
    _NSGetExecutablePath(NULL, &executable_length);
    ASSERT(executable_length > 1);

    FilePath executable_path;
    char* data = executable_path.value.appendCharactersUninitialized(
      static_cast<int>(executable_length - 1));
    int rv = _NSGetExecutablePath(writer.data(), &executable_length);
    ASSERT_UNUSED(rv == 0, rv)

    // _NSGetExecutablePath may return paths containing ./ or ../ which makes
    // FilePath::GetDirectoryName() work incorrectly, convert it to absolute path so that
    // paths such as DIR_SOURCE_ROOT can work, since we expect absolute paths to
    // be returned here.
    return File::MakeAbsolutePath(executable_path);
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveFile(g_key, provider);
}

FilePath GetAppUserDataPath() {
  auto provider = []() {
    return appendAppName(darwin::GetUserDirectory(NSApplicationSupportDirectory));
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::EnsureCreated);
}

FilePath GetAppCachePath() {
  auto provider = []() {
    return appendAppName(darwin::GetUserDirectory(NSCachesDirectory));
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::EnsureCreated);
}

FilePath GetUserDesktopPath() {
  #if OS(IOS)
  throw NotSupportedException("iOS does not have desktop directories");
  #else
  auto provider = []() {
    return darwin::GetUserDirectory(NSDesktopDirectory);
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::NotValidated);
  #endif
}

FilePath GetSourceTreePath() {
  // Start with the executable's directory.
  FilePath result = GetExecutableFilePath();
  #if !OS(IOS)
  int levels_up;
  if (mac::AmIBundled()) {
    // src/xcodebuild/{Debug|Release}/Application.app/Contents/MacOS/Application
    levels_up = 5;
  } else {
    // Unit tests execute two levels deep from the source root, eg:
    // src/xcodebuild/{Debug|Release}/BaseUnitTests
    levels_up = 2;
  }
  for (int i = 0; i < levels_up; ++i)
    result.CdUp();
  #endif
  return result;
}

} // namespace stp
