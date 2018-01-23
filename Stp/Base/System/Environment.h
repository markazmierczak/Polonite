// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_SYSTEM_ENVIRONMENT_H_
#define STP_BASE_SYSTEM_ENVIRONMENT_H_

#include "Base/Compiler/Os.h"
#include "Base/Containers/List.h"

namespace stp {

class BasicLock;
class FilePath;

class BASE_EXPORT Environment {
  STATIC_ONLY(Environment);
 public:
  static bool TryGet(StringSpan name, String& out_string);
  static bool TryGet(StringSpan name, int& out_value);
  static bool TryGet(StringSpan name, int64_t& out_value);
  static bool TryGet(StringSpan name, double& out_value);
  static bool TryGet(StringSpan name, FilePath& out_path);

  static bool TrySet(StringSpan name, StringSpan value);
  static bool TrySet(StringSpan name, const FilePath& path);

  static bool TryUnset(StringSpan name);

  static bool Has(StringSpan name);

  #if OS(WIN)
  static bool TryGetNative(const wchar_t* name, WString& out_string);
  static bool TryGet(const wchar_t* name, FilePath& out_path);
  static bool Has(const wchar_t* name);
  #elif OS(POSIX)
  static bool TryGetNative(const char* name, String& out_string);
  static bool TryGet(const char* name, String& out_string);
  static bool TryGet(const char* name, FilePath& out_path);
  static bool TrySet(const char* name, const char* value);
  static bool TryUnset(const char* name);
  static bool Has(const char* name);
  #endif

 private:
  #if OS(POSIX)
  // On POSIX getenv/setenv is not thread-safe.
  static BasicLock g_lock_;
  #endif
};

} // namespace stp

#endif // STP_BASE_SYSTEM_ENVIRONMENT_H_
