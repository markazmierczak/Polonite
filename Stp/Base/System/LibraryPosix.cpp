// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/Library.h"

#include "Base/Containers/Join.h"
#include "Base/FileSystem/FilePath.h"
#include "Base/Text/Format.h"

#include <dlfcn.h>

namespace stp {

static inline StringSpan DynamicLinkerErrorMessage() {
  return MakeSpanFromNullTerminated(dlerror());
}

void LibraryLoadError::FormatImpl(TextWriter& out) const {
  out << message_;
}

NativeLibrary Library::TryLoadNative(const FilePathChar* path, LibraryLoadError* out_error) {
  void* dl = dlopen(path, RTLD_LAZY);
  if (!dl) {
    if (out_error)
      out_error->message_ = DynamicLinkerErrorMessage();
  }
  return dl;
}

void Library::UnloadNative(NativeLibrary library) {
  int ret = dlclose(library);
  if (ret != 0)
    ASSERT(false, "failed to unload library: {}", DynamicLinkerErrorMessage());
}

void* Library::TryResolveNative(NativeLibrary library, const char* name) {
  return dlsym(library, name);
}

String Library::DecorateName(StringSpan name) {
  ASSERT(IsAscii(name));
  return ConcatMany<String>("lib", name, ".so");
}

} // namespace stp
