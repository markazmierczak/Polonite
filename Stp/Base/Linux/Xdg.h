// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_LINUX_XDG_H_
#define STP_BASE_LINUX_XDG_H_

#include "Base/Containers/List.h"
#include "Base/FileSystem/FilePath.h"

namespace stp {
namespace linux {

enum DesktopEnvironment {
  Other,
  Gnome,
  // KDE3, KDE4 and KDE5 are sufficiently different that we count
  // them as different desktop environments here.
  Kde3,
  Kde4,
  Kde5,
  Unity,
  Xfce,
};

class BASE_EXPORT Xdg {
  STATIC_ONLY(Xdg);
 public:
  static constexpr char DotConfigDir[] = ".config";
  static constexpr char ConfigHomeEnvVar[] = "XDG_CONFIG_HOME";

  // Utility function for getting XDG directories.
  // |env_name| is the name of an environment variable that we want to use to get
  // a directory path. |fallback_dir| is the directory relative to $HOME that we
  // use if |env_name| cannot be found or is empty. |fallback_dir| may be null.
  // Examples of |env_name| are XDG_CONFIG_HOME and XDG_DATA_HOME.
  static FilePath GetDirectory(const char* env_name, StringSpan fallback_dir);

  // This looks up "well known" user directories like the desktop and music
  // folder. Examples of |dir_name| are DESKTOP and MUSIC.
  static FilePath GetUserDirectory(const char* dir_name, StringSpan fallback_dir);

  // Return an entry from the DesktopEnvironment enum with a best guess
  // of which desktop environment we're using.
  static DesktopEnvironment GetDesktopEnvironment();
};

} // namespace linux
} // namespace stp

#endif // STP_BASE_LINUX_XDG_H_
