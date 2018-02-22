// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_KNOWNPATHS_H_
#define STP_BASE_FS_KNOWNPATHS_H_

#include "Base/FileSystem/FilePath.h"

namespace stp {

// Root directory for temporary files (usually /tmp on Linux).
BASE_EXPORT FilePath getTempDirPath();

// User's root home directory.
// On Windows this will look like "C:\Users\<user>"
// which isn't necessarily a great place to put files.
BASE_EXPORT FilePath getHomeDirPath();

// gets/Sets the current working directory for the process.
BASE_EXPORT FilePath getCurrentDirPath();
BASE_EXPORT bool setCurrentDirPath(const FilePath& path);

// Directory where user data can be written.
BASE_EXPORT FilePath getAppUserDataPath();

// Directory where user specific non-essential data files should be stored.
BASE_EXPORT FilePath getAppCachePath();

// The current user's Desktop.
BASE_EXPORT FilePath getUserDesktopPath();

// Useful for tests that need to locate various resources.
// It should not be used outside of test code.
BASE_EXPORT FilePath getSourceTreePath();
BASE_EXPORT FilePath getBaseTestDataPath();

// Path to the current executable and its directory.
BASE_EXPORT FilePath getExecutableFilePath();
BASE_EXPORT FilePath getExecutableDirPath();

#if OS(ANDROID)

// Directory where to put Android app's data.
BASE_EXPORT FilePath getAndroidAppDataPath();
// Android external storage directory.
BASE_EXPORT FilePath getAndroidExternalStoragePath();

#elif OS(DARWIN)

// ~/Library/Application Support directory
BASE_EXPORT FilePath getDarwinAppDataPath();

#elif OS(WIN)

// Windows directory, usually "C:\Windows"
BASE_EXPORT FilePath getWindowsPath();

BASE_EXPORT FilePath getProgramFilesPath();

// Application Data directory under the user profile.
BASE_EXPORT FilePath getWinAppDataPath();

// "Local Settings\Application Data" directory under the user profile.
BASE_EXPORT FilePath getWinLocalAppDataPath();

// Directory for the common desktop (visible // on all user's Desktop).
BASE_EXPORT FilePath getWinCommonDesktopPath();

// Usually "C:\ProgramData\Microsoft\Windows\Start Menu\Programs"
BASE_EXPORT FilePath getWinCommonStartMenuPath();

// Usually "C:\Users\<user>\AppData\Roaming\Microsoft\Windows\Start Menu\Programs"
BASE_EXPORT FilePath getWinStartMenuPath();

// Usually C:\Windows\Fonts.
BASE_EXPORT FilePath getWinFontsPath();

#endif // OS(*)

} // namespace stp

#endif // STP_BASE_FS_KNOWNPATHS_H_
