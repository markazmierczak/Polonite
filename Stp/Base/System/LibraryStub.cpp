// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/Library.h"

namespace stp {

void LibraryLoadError::formatImpl(TextWriter& out) const {
  out.Write(message_);
}

NativeLibrary Library::TryLoadNative(const FilePathChar* library_path, LibraryLoadError* out_error) {
  out_error->message_ = "not supported";
  return nullptr;
}

void Library::UnloadNative(NativeLibrary library) {
  ASSERT(false, "not implemented");
}

void* Library::TryResolveNative(NativeLibrary library, const char* name) {
  ASSERT(false, "not implemented");
  return nullptr;
}

String Library::DecorateName(StringSpan name) {
  ASSERT(name.isAscii());
  return ToString(name);
}

} // namespace stp
