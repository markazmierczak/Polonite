// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Exe/Library.h"

#include "Base/Fs/FilePath.h"
#include "Base/Text/Format.h"

#include <dlfcn.h>

namespace stp {

static inline StringSpan DynamicLinkerErrorMessage() {
  return MakeSpanFromNullTerminated(dlerror());
}

void LibraryLoadError::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out.Write(message);
}

NativeLibrary Library::TryLoadNative(const FilePathChar* path, LibraryLoadError* out_error) {
  void* dl = dlopen(path, RTLD_LAZY);
  if (!dl) {
    if (out_error)
      out_error->message = DynamicLinkerErrorMessage();
  }
  return dl;
}

void Library::UnloadNative(NativeLibrary library) {
  int ret = dlclose(library);
  if (ret != 0)
    ASSERT(false, "failed to unload library: {}", DynamicLinkerErrorMessage());
}

void* Library::ResolveNative(NativeLibrary library, const char* name) {
  return dlsym(library, name);
}

String Library::DecorateName(StringSpan name) {
  ASSERT(IsAscii(name));
  return String::ConcatArgs("lib", name, ".so");
}

} // namespace stp
