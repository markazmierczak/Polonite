// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Fs/KnownPaths.h"

#include "Base/App/Application.h"
#include "Base/Fs/Directory.h"
#include "Base/Fs/KnownPathUtil.h"
#include "Base/System/SysInfo.h"
#include "Base/Error/SystemException.h"
#include "Base/Win/CurrentModule.h"
#include "Base/Win/ScopedCoMem.h"

#include <windows.h>
#include <shlobj.h>

namespace stp {

static inline FilePath AppendAppName(FilePath&& path) {
  path.AppendAscii(Application::Instance().GetName());
  return path;
}

FilePath GetTempDirPath() {
  auto provider = []() {
    wchar_t path[MAX_PATH + 1];
    DWORD length = ::GetTempPathW(MAX_PATH, path);
    if (length == 0)
      throw SystemException(GetLastWinErrorCode());
    return FilePath(path, static_cast<int>(length));
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::NotValidated);
}

FilePath GetCurrentDirPath() {
  // Cannot be cached for obvious reasons.
  FilePath path;
  int buffer_length = 1;
  while (true) {
    wchar_t* dst = path.AppendCharactersUninitialized(buffer_length - 1);
    int rv = static_cast<int>(::GetCurrentDirectoryW(buffer_length, dst));
    if (rv < buffer_length) {
      if (rv == 0)
        throw SystemException(GetLastWinErrorCode());
      path.TruncateCharacters(rv);
      return path;
    }
    buffer_length = rv;
    path.Clear();
  }
}

bool SetCurrentDirPath(const FilePath& directory) {
  return ::SetCurrentDirectoryW(ToNullTerminated(directory)) != 0;
}

FilePath GetAppUserDataPath() {
  auto provider = []() {
    return AppendAppName(GetWinLocalAppDataPath());
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::EnsureCreated);
}

FilePath GetAppCachePath() {
  auto provider = []() {
    // Windows has no notion of cache directory.
    // Use subdir in application user data.
    FilePath path = GetAppUserDataPath();
    path.AppendAscii("Cache");
    return path;
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::EnsureCreated);
}

static FilePath GetModuleFile(HMODULE module) {
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
    wchar_t* dst = path.AppendCharactersUninitialized(capacity - 1);
    rv = static_cast<int>(::GetModuleFileNameW(module, dst, capacity));
    if (rv < capacity) {
      path.TruncateCharacters(rv);
      return path;
    }
    path.Clear();
  }
  throw SystemException(GetLastWinErrorCode());
}

FilePath GetExecutableFilePath() {
  auto provider = []() { return GetModuleFile(0); };
  static known_path::Key g_key = 0;
  return known_path::ResolveFile(g_key, provider, known_path::NotValidated);
}

FilePath GetWindowsPath() {
  auto provider = []() {
    wchar_t path[MAX_PATH];
    int rv = static_cast<int>(::GetWindowsDirectoryW(path, MAX_PATH));
    ASSERT(rv != 0 && rv <= MAX_PATH);
    return FilePath(path, rv);
  };
  static known_path::Key g_key = 0;
  return known_path::ResolveDirectory(g_key, provider, known_path::NotValidated);
}

static FilePath ShGetKnownFolderPathWrapper(REFKNOWNFOLDERID rfid) {
  win::ScopedCoMem<wchar_t> path_buf;
  HRESULT rv = ::SHGetKnownFolderPath(rfid, 0, NULL, &path_buf);
  if (rv == S_OK)
    return MakeFilePathFromNullTerminated(path_buf.get());
  throw ComException(rv) << "failed to resolve known path";
}

#define DEFINE_SHELL_BASED_FOLDER(NAME, FOLDERID) \
  FilePath NAME() { \
    auto provider = []() { \
      return ShGetKnownFolderPathWrapper(FOLDERID); \
    }; \
    static known_path::Key g_key = 0; \
    return known_path::ResolveDirectory(g_key, provider, known_path::NotValidated); \
  }

DEFINE_SHELL_BASED_FOLDER(GetHomeDirPath, FOLDERID_Profile)
DEFINE_SHELL_BASED_FOLDER(GetProgramFilesPath, FOLDERID_ProgramFiles)
DEFINE_SHELL_BASED_FOLDER(GetWinAppDataPath, FOLDERID_RoamingAppData)
DEFINE_SHELL_BASED_FOLDER(GetWinLocalAppDataPath, FOLDERID_LocalAppData)
DEFINE_SHELL_BASED_FOLDER(GetWinCommonDesktopPath, FOLDERID_PublicDesktop)
DEFINE_SHELL_BASED_FOLDER(GetWinUserDesktopPath, FOLDERID_Desktop)
DEFINE_SHELL_BASED_FOLDER(GetWinCommonStartMenuPath, FOLDERID_CommonPrograms)
DEFINE_SHELL_BASED_FOLDER(GetWinStartMenuPath, FOLDERID_Programs)
DEFINE_SHELL_BASED_FOLDER(GetWinFontsPath, FOLDERID_Fonts)

#undef DEFINE_SHELL_BASED_FOLDER

} // namespace stp
