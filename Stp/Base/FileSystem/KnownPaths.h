// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_KNOWNPATHS_H_
#define STP_BASE_FS_KNOWNPATHS_H_

#include "Base/FileSystem/FilePath.h"

namespace stp {

// Root directory for temporary files (usually /tmp on Linux).
BASE_EXPORT FilePath GetTempDirPath();

// User's root home directory.
// On Windows this will look like "C:\Users\<user>"
// which isn't necessarily a great place to put files.
BASE_EXPORT FilePath GetHomeDirPath();

// Gets/Sets the current working directory for the process.
BASE_EXPORT FilePath GetCurrentDirPath();
BASE_EXPORT bool SetCurrentDirPath(const FilePath& path);

// Directory where user data can be written.
BASE_EXPORT FilePath GetAppUserDataPath();

// Directory where user specific non-essential data files should be stored.
BASE_EXPORT FilePath GetAppCachePath();

// The current user's Desktop.
BASE_EXPORT FilePath GetUserDesktopPath();

// Useful for tests that need to locate various resources.
// It should not be used outside of test code.
BASE_EXPORT FilePath GetSourceTreePath();
BASE_EXPORT FilePath GetBaseTestDataPath();

// Path to the current executable and its directory.
BASE_EXPORT FilePath GetExecutableFilePath();
BASE_EXPORT FilePath GetExecutableDirPath();

#if OS(ANDROID)

// Directory where to put Android app's data.
BASE_EXPORT FilePath GetAndroidAppDataPath();
// Android external storage directory.
BASE_EXPORT FilePath GetAndroidExternalStoragePath();

#elif OS(DARWIN)

// ~/Library/Application Support directory
BASE_EXPORT FilePath GetDarwinAppDataPath();

#elif OS(WIN)

// Windows directory, usually "C:\Windows"
BASE_EXPORT FilePath GetWindowsPath();

BASE_EXPORT FilePath GetProgramFilesPath();

// Application Data directory under the user profile.
BASE_EXPORT FilePath GetWinAppDataPath();

// "Local Settings\Application Data" directory under the user profile.
BASE_EXPORT FilePath GetWinLocalAppDataPath();

// Directory for the common desktop (visible // on all user's Desktop).
BASE_EXPORT FilePath GetWinCommonDesktopPath();

// Usually "C:\ProgramData\Microsoft\Windows\Start Menu\Programs"
BASE_EXPORT FilePath GetWinCommonStartMenuPath();

// Usually "C:\Users\<user>\AppData\Roaming\Microsoft\Windows\Start Menu\Programs"
BASE_EXPORT FilePath GetWinStartMenuPath();

// Usually C:\Windows\Fonts.
BASE_EXPORT FilePath GetWinFontsPath();

#endif // OS(*)

} // namespace stp

#endif // STP_BASE_FS_KNOWNPATHS_H_
