// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/KnownPaths.h"

#include "Base/App/Application.h"
#include "Base/FileSystem/Directory.h"
#include "Base/FileSystem/File.h"
#include "Base/FileSystem/KnownPathUtil.h"
#include "Base/Linux/ProcCommon.h"
#include "Base/Linux/Xdg.h"
#include "Base/Process/NativeProcess.h"

namespace stp {

using linux::Xdg;

static FilePath addAppName(FilePath path) {
  path.addComponentAscii(Application::instance().getName());
  return path;
}

FilePath getExecutableFilePath() {
  auto provider = []() {
    return NativeProcess::getExecutablePath(NativeProcess::getCurrentHandle());
  };
  static known_path::Key g_key = 0;
  return known_path::resolveFile(g_key, provider, known_path::NotValidated);
}

FilePath getUserDesktopPath() {
  auto provider = []() {
    return Xdg::getUserDirectory("DESKTOP", "Desktop");
  };
  static known_path::Key g_key = 0;
  return known_path::resolveDirectory(g_key, provider, known_path::NotValidated);
}

FilePath getAppUserDataPath() {
  // See http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
  auto provider = []() {
    return addAppName(Xdg::getDirectory(Xdg::ConfigHomeEnvVar, Xdg::DotConfigDir));
  };
  static known_path::Key g_key = 0;
  return known_path::resolveDirectory(g_key, provider, known_path::EnsureCreated);
}

FilePath getAppCachePath() {
  auto provider = []() {
    return addAppName(Xdg::getDirectory("XDG_CACHE_HOME", ".cache"));
  };
  static known_path::Key g_key = 0;
  return known_path::resolveDirectory(g_key, provider, known_path::EnsureCreated);
}

} // namespace stp
