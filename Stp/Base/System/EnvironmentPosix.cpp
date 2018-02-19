// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/Environment.h"

#include "Base/FileSystem/FilePath.h"
#include "Base/Text/Wtf.h"
#include "Base/Thread/Lock.h"

#include <stdlib.h>

namespace stp {

BasicLock Environment::g_lock_ = BASIC_LOCK_INITIALIZER;

bool Environment::TryGet(StringSpan name, String& out_value) {
  return TryGet(ToNullTerminated(String(name)), out_value);
}

bool Environment::TryGet(const char* name, String& out_value) {
  #if HAVE_UTF8_NATIVE_VALIDATION
  if (!TryGetNative(name, out_value))
    return false;
  ASSERT(Utf8::Validate(out_value));
  #else
  String mbvalue;
  if (!TryGetNative(name, mbvalue))
    return false;
  out_value.clear();
  AppendWtf(out_value, mbvalue);
  #endif
  return true;
}

bool Environment::TryGetNative(const char* name, String& out_value) {
  AutoLock auto_lock(&g_lock_);
  const char* mbvalue_cstr = ::getenv(name);
  if (!mbvalue_cstr)
    return false;
  out_value = makeSpanFromNullTerminated(mbvalue_cstr);
  return true;
}

bool Environment::TryGet(StringSpan name, FilePath& out_path) {
  return TryGet(ToNullTerminated(String(name)), out_path);
}

bool Environment::TryGet(const char* name, FilePath& out_path) {
  return TryGetNative(name, out_path.chars());
}

bool Environment::Has(StringSpan name) {
  return Has(ToNullTerminated(String(name)));
}

bool Environment::Has(const char* name) {
  ASSERT(name);
  AutoLock auto_lock(&g_lock_);
  return ::getenv(name) != nullptr;
}

bool Environment::TrySet(StringSpan name, StringSpan value) {
  return TrySet(ToNullTerminated(String(name)), ToNullTerminated(String(value)));
}

bool Environment::TrySet(const char* name, const char* value) {
  AutoLock auto_lock(&g_lock_);
  return ::setenv(name, value, 1) == 0;
}

bool Environment::TrySet(StringSpan name, const FilePath& path) {
  return TrySet(ToNullTerminated(String(name)), ToNullTerminated(path));
}

bool Environment::TryUnset(StringSpan name) {
  return TryUnset(ToNullTerminated(String(name)));
}

bool Environment::TryUnset(const char* name) {
  AutoLock auto_lock(&g_lock_);
  return ::unsetenv(name) == 0;
}
} // namespace stp
