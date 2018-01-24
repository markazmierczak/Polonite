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

bool Environment::TryGet(StringSpan name, String& out_value) {
  WString wvalue;
  bool ok = TryGet(ToNullTerminated(ToWString(name)), wvalue);
  if (ok) {
    out_value.Clear();
    AppendWtf(out_value, wvalue);
  }
  return ok;
}

bool Environment::TryGetNative(const wchar_t* name, WString& out_value) {
  int size = 1; // including null terminator
  while (true) {
    out_value.Clear();
    wchar_t* dst = out_value.AppendUninitialized(size - 1);

    int rv = static_cast<int>(::GetEnvironmentVariableW(name, dst, size));
    if (rv == 0)
      return false;
    if (rv < size)
      break;
    size = rv;
  }
  return true;
}

bool Environment::TryGet(StringSpan name, FilePath& out_path) {
  return TryGet(ToNullTerminated(ToWString(name)), out_path);
}

bool Environment::TryGet(const wchar_t* name, FilePath& out_path) {
  return TryGetNative(name, out_path.chars());
}

bool Environment::Has(StringSpan name) {
  return Has(ToNullTerminated(ToWString(name)));
}

bool Environment::Has(const wchar_t* name) {
  return ::GetEnvironmentVariableW(name, nullptr, 0) != 0;
}

bool Environment::TrySet(StringSpan name, StringSpan value) {
  return TrySet(ToNullTerminated(ToWString(name)), ToNullTerminated(ToWString(value)));
}

bool Environment::TrySet(StringSpan name, const FilePath& path) {
  return TrySet(ToNullTerminated(ToWString(name)), ToNullTerminated(path));
}

bool Environment::TrySet(const wchar_t* name, const wchar_t* value) {
  return ::SetEnvironmentVariableW(name, value) != 0;
}

bool Environment::TryUnset(StringSpan name) {
  return TryUnset(ToNullTerminated(ToWString(name)));
}

bool Environment::TryUnset(const wchar_t* name) {
  return TrySet(name, nullptr);
}

} // namespace stp
