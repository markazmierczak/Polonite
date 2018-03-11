// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/KnownPaths.h"

#include "Base/App/Application.h"
#include "Base/FileSystem/Directory.h"
#include "Base/FileSystem/KnownPathUtil.h"
#include "Base/System/SysInfo.h"
#include "Base/Error/SystemException.h"
#include "Base/Win/CurrentModule.h"
#include "Base/Win/ScopedCoMem.h"

#include <windows.h>
#include <shlobj.h>

namespace stp {

static inline FilePath appendAppName(FilePath&& path) {
  path.appendAscii(Application::instance().getName());
  return path;
}

FilePath getTempDirPath() {
  auto provider = []() {
    wchar_t path[MAX_PATH + 1];
    DWORD length = ::GetTempPathW(MAX_PATH, path);
    if (length == 0)
      throw SystemException(lastWinErrorCode());
    return FilePath(path, static_cast<int>(length));
  };
  static known_path::Key g_key = 0;
  return known_path::resolveDirectory(g_key, provider, known_path::NotValidated);
}

FilePath getCurrentDirPath() {
  // Cannot be cached for obvious reasons.
  FilePath path;
  int buffer_length = 1;
  while (true) {
    wchar_t* dst = path.appendCharactersUninitialized(buffer_length - 1);
    int rv = static_cast<int>(::GetCurrentDirectoryW(buffer_length, dst));
    if (rv < buffer_length) {
      if (rv == 0)
        throw SystemException(lastWinErrorCode());
      path.truncateCharacters(rv);
      return path;
    }
    buffer_length = rv;
    path.clear();
  }
}

bool setCurrentDirPath(const FilePath& directory) {
  return ::SetCurrentDirectoryW(toNullTerminated(directory)) != 0;
}

FilePath getAppUserDataPath() {
  auto provider = []() {
    return appendAppName(getWinLocalAppDataPath());
  };
  static known_path::Key g_key = 0;
  return known_path::resolveDirectory(g_key, provider, known_path::EnsureCreated);
}

FilePath getAppCachePath() {
  auto provider = []() {
    // Windows has no notion of cache directory.
    // Use subdir in application user data.
    FilePath path = getAppUserDataPath();
    path.appendAscii("Cache");
    return path;
  };
  static known_path::Key g_key = 0;
  return known_path::resolveDirectory(g_key, provider, known_path::EnsureCreated);
}

static FilePath getModuleFile(HMODULE module) {
  static constexpr int StackCapacity = 256;
  wchar_t stack_buffer[StackCapacity];

  int rv = static_cast<int>(::GetModuleFileNameW(module, stack_buffer, StackCapacity));
  if (rv < StackCapacity) {
    // stack buffer was enough in size, quick path
    return FilePath(stack_buffer, rv);
  }
  // stack buffer was not enough, allocate on heap
  FilePath path;
  for (int capacity = StackCapacity * 2; rv != 0; capacity *= 2) {
    wchar_t* dst = path.appendCharactersUninitialized(capacity - 1);
    rv = static_cast<int>(::GetModuleFileNameW(module, dst, capacity));
    if (rv < capacity) {
      path.truncateCharacters(rv);
      return path;
    }
    path.clear();
  }
  throw SystemException(lastWinErrorCode());
}

FilePath getExecutableFilePath() {
  auto provider = []() { return getModuleFile(0); };
  static known_path::Key g_key = 0;
  return known_path::ResolveFile(g_key, provider, known_path::NotValidated);
}

FilePath getWindowsPath() {
  auto provider = []() {
    wchar_t path[MAX_PATH];
    int rv = static_cast<int>(::GetWindowsDirectoryW(path, MAX_PATH));
    ASSERT(rv != 0 && rv <= MAX_PATH);
    return FilePath(path, rv);
  };
  static known_path::Key g_key = 0;
  return known_path::resolveDirectory(g_key, provider, known_path::NotValidated);
}

static FilePath shGetKnownFolderPathWrapper(REFKNOWNFOLDERID rfid) {
  win::ScopedCoMem<wchar_t> path_buf;
  HRESULT rv = ::SHGetKnownFolderPath(rfid, 0, NULL, &path_buf);
  if (rv == S_OK)
    return MakeFilePathFromNullTerminated(path_buf.get());
  throw ComException(rv) << "failed to resolve known path";
}

#define DEFINE_SHELL_BASED_FOLDER(NAME, FOLDERID) \
  FilePath NAME() { \
    auto provider = []() { \
      return shGetKnownFolderPathWrapper(FOLDERID); \
    }; \
    static known_path::Key g_key = 0; \
    return known_path::ResolveDirectory(g_key, provider, known_path::NotValidated); \
  }

DEFINE_SHELL_BASED_FOLDER(getHomeDirPath, FOLDERID_Profile)
DEFINE_SHELL_BASED_FOLDER(getProgramFilesPath, FOLDERID_ProgramFiles)
DEFINE_SHELL_BASED_FOLDER(getWinAppDataPath, FOLDERID_RoamingAppData)
DEFINE_SHELL_BASED_FOLDER(getWinLocalAppDataPath, FOLDERID_LocalAppData)
DEFINE_SHELL_BASED_FOLDER(getWinCommonDesktopPath, FOLDERID_PublicDesktop)
DEFINE_SHELL_BASED_FOLDER(getWinUserDesktopPath, FOLDERID_Desktop)
DEFINE_SHELL_BASED_FOLDER(getWinCommonStartMenuPath, FOLDERID_CommonPrograms)
DEFINE_SHELL_BASED_FOLDER(getWinStartMenuPath, FOLDERID_Programs)
DEFINE_SHELL_BASED_FOLDER(getWinFontsPath, FOLDERID_Fonts)

#undef DEFINE_SHELL_BASED_FOLDER

} // namespace stp
