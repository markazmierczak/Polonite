// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/Library.h"

#include "Base/Io/TextWriter.h"
#include "Base/Win/WinErrorCode.h"

namespace stp {

void LibraryLoadError::formatImpl(TextWriter& out) const {
  out << static_cast<WinErrorCode>(code_);
}

static inline bool AreSearchFlagsAvailable() {
  using AddDllDirectoryFunction = HMODULE (*)(PCWSTR new_directory);

  // The LOAD_LIBRARY_SEARCH_* flags are available on systems that have
  // KB2533623 installed. To determine whether the flags are available, use
  // GetProcAddress to get the address of the AddDllDirectory,
  // RemoveDllDirectory, or SetDefaultDllDirectories function. If GetProcAddress
  // succeeds, the LOAD_LIBRARY_SEARCH_* flags can be used with LoadLibraryEx.
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms684179(v=vs.85).aspx
  // The LOAD_LIBRARY_SEARCH_* flags are used in the LoadNativeLibraryHelper method.
  auto add_dll_dir_func = reinterpret_cast<AddDllDirectoryFunction>(
      GetProcAddress(GetModuleHandle(L"kernel32.dll"), "AddDllDirectory"));
  return add_dll_dir_func != nullptr;
}

NativeLibrary Library::TryLoadNative(const FilePathChar* path, LibraryLoadError* out_error) {
  NativeLibrary module = NullNativeLibrary;

  bool are_search_flags_available = AreSearchFlagsAvailable();
  if (are_search_flags_available) {
    // LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR flag is needed to search the library
    // directory as the library may have dependencies on DLLs in this directory.
    module = ::LoadLibraryExW(
        path, nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (module)
      return module;
  }

  module = ::LoadLibraryW(path);

  if (!module) {
    if (out_error)
      out_error->code_ = ::GetLastError();
  }
  return module;
}

void Library::UnloadNative(NativeLibrary library) {
  ::FreeLibrary(library);
}

void* Library::TryResolveNative(NativeLibrary library, const char* name) {
  return ::GetProcAddress(library, name);
}

String Library::DecorateName(StringSpan name) {
  ASSERT(name.isAscii());
  return String::ConcatArgs(name, ".dll");
}

} // namespace stp
