// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/Environment.h"

#include "Base/FileSystem/FilePath.h"
#include "Base/Text/Wtf.h"

// Variables in Windows environment are stored as wide string internally.
// The C runtime has internal COPY of this area.
// Thus we use Win32 API directly - (Get/Set)EnvironmentVariable.

#include <windows.h>

namespace stp {

bool Environment::tryGet(StringSpan name, String& out_value) {
  WString wvalue;
  bool ok = tryGet(toNullTerminated(ToWString(name)), wvalue);
  if (ok) {
    out_value.clear();
    appendWtf(out_value, wvalue);
  }
  return ok;
}

bool Environment::tryGetNative(const wchar_t* name, WString& out_value) {
  int size = 1; // including null terminator
  while (true) {
    out_value.clear();
    wchar_t* dst = out_value.appendUninitialized(size - 1);

    int rv = static_cast<int>(::GetEnvironmentVariableW(name, dst, size));
    if (rv == 0)
      return false;
    if (rv < size)
      break;
    size = rv;
  }
  return true;
}

bool Environment::tryGet(StringSpan name, FilePath& out_path) {
  return tryGet(toNullTerminated(ToWString(name)), out_path);
}

bool Environment::tryGet(const wchar_t* name, FilePath& out_path) {
  return tryGetNative(name, out_path.chars());
}

bool Environment::Has(StringSpan name) {
  return Has(toNullTerminated(ToWString(name)));
}

bool Environment::Has(const wchar_t* name) {
  return ::GetEnvironmentVariableW(name, nullptr, 0) != 0;
}

bool Environment::TrySet(StringSpan name, StringSpan value) {
  return TrySet(toNullTerminated(ToWString(name)), toNullTerminated(ToWString(value)));
}

bool Environment::TrySet(StringSpan name, const FilePath& path) {
  return TrySet(toNullTerminated(ToWString(name)), toNullTerminated(path));
}

bool Environment::TrySet(const wchar_t* name, const wchar_t* value) {
  return ::SetEnvironmentVariableW(name, value) != 0;
}

bool Environment::TryUnset(StringSpan name) {
  return TryUnset(toNullTerminated(ToWString(name)));
}

bool Environment::TryUnset(const wchar_t* name) {
  return TrySet(name, nullptr);
}

} // namespace stp
