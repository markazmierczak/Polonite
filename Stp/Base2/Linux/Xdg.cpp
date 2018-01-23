// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Linux/Xdg.h"

#include "Base/Fs/KnownPaths.h"
#include "Base/System/Environment.h"
#include "Base/ThirdParty/xdg_user_dirs/xdg_user_dir_lookup.h"

namespace stp {
namespace linux {

constexpr char Xdg::ConfigHomeEnvVar[];
constexpr char Xdg::DotConfigDir[];

FilePath Xdg::GetDirectory(const char* env_name, StringSpan fallback_dir) {
  FilePath path;
  String env_value;
  if (Environment::TryGet(env_name, path)) {
    path.StripTrailingSeparators();
  } else {
    path = GetHomeDirPath();
    path.AddAscii(fallback_dir);
  }
  return path;
}

FilePath Xdg::GetUserDirectory(const char* dir_name, StringSpan fallback_dir) {
  FilePath path;
  char* xdg_dir = xdg_user_dir_lookup(dir_name);
  if (xdg_dir) {
    path = MakeFilePathSpanFromNullTerminated(xdg_dir);
    free(xdg_dir);
    path.StripTrailingSeparators();
  } else {
    path = GetHomeDirPath();
    path.AddAscii(fallback_dir);
  }
  return path;
}

/* FIXME
static DesktopEnvironment GetKdeDesktopEnvironment() {
  // The KDE session version environment variable introduced in KDE 4.
  int kde_session;
  if (Environment::TryGet("KDE_SESSION_VERSION", kde_session)) {
    if (kde_session >= 5)
      return DesktopEnvironment::Kde5;
    return DesktopEnvironment::Kde4;
  }
  return DesktopEnvironment::Kde3;
} */

DesktopEnvironment Xdg::GetDesktopEnvironment() {
  // XDG_CURRENT_DESKTOP is the newest standard circa 2012.
  /* FIXME String xdg_current_desktop;
  if (Environment::TryGet("XDG_CURRENT_DESKTOP", xdg_current_desktop)) {
    // Not all desktop environments set this env var as of this writing.
    if (StartsWith(xdg_current_desktop, "Unity")) {
      // gnome-fallback sessions set XDG_CURRENT_DESKTOP to Unity
      // DESKTOP_SESSION can be gnome-fallback or gnome-fallback-compiz
      String desktop_session;
      if (Environment::TryGet("DESKTOP_SESSION", desktop_session) &&
          ContainsRange(desktop_session, "gnome-fallback")) {
        return DesktopEnvironment::Gnome;
      }
      return DesktopEnvironment::Unity;
    }

    if (xdg_current_desktop == "GNOME")
      return DesktopEnvironment::Gnome;

    if (xdg_current_desktop == "KDE")
      return GetKdeDesktopEnvironment();
  }

  // DESKTOP_SESSION was what everyone used in 2010.
  String desktop_session;
  if (Environment::TryGet("DESKTOP_SESSION", desktop_session)) {
    if (desktop_session == "gnome" || desktop_session =="mate")
      return DesktopEnvironment::Gnome;
    if (desktop_session == "kde4" || desktop_session == "kde-plasma")
      return DesktopEnvironment::Kde4;
    if (desktop_session == "kde")
      return GetKdeDesktopEnvironment();
    if (ContainsRange(desktop_session, "xfce") || desktop_session == "xubuntu")
      return DesktopEnvironment::Xfce;
  }

  // Fall back on some older environment variables.
  // Useful particularly in the DESKTOP_SESSION=default case.
  if (Environment::Has("GNOME_DESKTOP_SESSION_ID"))
    return DesktopEnvironment::Gnome;

  if (Environment::Has("KDE_FULL_SESSION"))
    return GetKdeDesktopEnvironment(); */
  return DesktopEnvironment::Other;
}

} // namespace linux
} // namespace stp
