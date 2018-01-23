// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Fs/KnownPaths.h"

#include "Base/App/Application.h"
#include "Base/Fs/Directory.h"
#include "Base/Fs/File.h"
#include "Base/Fs/KnownPathUtil.h"
#include "Base/Linux/ProcCommon.h"
#include "Base/Linux/Xdg.h"
#include "Base/Process/NativeProcess.h"

namespace stp {

using linux::Xdg;

static FilePath AddAppName(FilePath path) {
  path.AddAscii(Application::Instance().GetName());
  return path;
}

FilePath GetExecutableFilePath() {
  auto provider = []() {
    return NativeProcess::GetExecutablePath(NativeProcess::GetCurrentHandle());
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveFile(g_key, provider, known_path::NotValidated);
}

FilePath GetUserDesktopPath() {
  auto provider = []() {
    return Xdg::GetUserDirectory("DESKTOP", "Desktop");
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::NotValidated);
}

FilePath GetAppUserDataPath() {
  // See http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
  auto provider = []() {
    return AddAppName(Xdg::GetDirectory(
        Xdg::ConfigHomeEnvVar, Xdg::DotConfigDir));
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::EnsureCreated);
}

FilePath GetAppCachePath() {
  auto provider = []() {
    return AddAppName(Xdg::GetDirectory("XDG_CACHE_HOME", ".cache"));
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::EnsureCreated);
}

} // namespace stp
