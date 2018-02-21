// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_SYSTEM_LIBRARY_H_
#define STP_BASE_SYSTEM_LIBRARY_H_

#include "Base/FileSystem/FilePath.h"

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#elif OS(DARWIN)
#import <CoreFoundation/CoreFoundation.h>
#endif

namespace stp {

#if OS(WIN)
using NativeLibrary = HMODULE;
#elif OS(DARWIN)
enum class NativeLibraryType {
  Bundle,
  DynamicLib,
};
enum class NativeLibraryObjCStatus {
  Unknown,
  Present,
  NotPresent,
};
struct NativeLibraryStruct {
  NativeLibraryType type;
  NativeLibraryObjCStatus objc_status;
  CFBundleRefNum bundle_resource_ref;
  union {
    CFBundleRef bundle;
    void* dylib;
  };
};
using NativeLibrary = NativeLibraryStruct*;
#elif OS(POSIX)
using NativeLibrary = void*;
#endif // OS(*)
constexpr NativeLibrary NullNativeLibrary = nullptr;

class BASE_EXPORT LibraryLoadError {
 public:
  LibraryLoadError() = default;

  friend TextWriter& operator<<(TextWriter& out, const LibraryLoadError& x) {
    x.formatImpl(out); return out;
  }
  friend void format(TextWriter& out, const LibraryLoadError& x, const StringSpan& opts) {
    x.formatImpl(out);
  }

 private:
  friend class Library;

  #if OS(WIN)
  DWORD code_ = 0;
  #else
  String message_;
  #endif // OS(*)

  void formatImpl(TextWriter& out) const;
};

// Platform-independent library type which represents a loadable module.
class BASE_EXPORT Library {
 public:
  Library() {}

  ~Library() { Reset(); }

  bool TryLoad(const FilePath& path, LibraryLoadError* error = nullptr);

  explicit Library(NativeLibrary native) : native_(native) {}

  Library(Library&& other) : native_(exchange(other.native_, NullNativeLibrary)) {}
  Library& operator=(Library&& other) { Reset(other.release()); return *this; }

  friend void swap(Library& lhs, Library& rhs) { swap(lhs.native_, rhs.native_); }

  void Reset(NativeLibrary native = NullNativeLibrary);

  ALWAYS_INLINE NativeLibrary get() const { return native_; }

  void* TryResolve(const char* function_name) const {
    return TryResolveNative(native_, function_name);
  }

  // Returns true if there's a valid library loaded.
  bool IsValid() const { return native_ != NullNativeLibrary; }

  // Returns the native library handle and removes it from this object.
  // The caller must manage the lifetime of the handle.
  NativeLibrary release();

  // Returns the full platform specific name for a native library.
  // |name| must be ASCII.
  // For example:
  // "mylib" returns "mylib.dll" on Windows, "libmylib.so" on Linux,
  // "libmylib.dylib" on Mac.
  static String DecorateName(StringSpan name);

 private:
  NativeLibrary native_ = NullNativeLibrary;

  static NativeLibrary TryLoadNative(const FilePathChar* path, LibraryLoadError* error);
  static void UnloadNative(NativeLibrary library);
  static void* TryResolveNative(NativeLibrary library, const char* function_name);

  DISALLOW_COPY_AND_ASSIGN(Library);
};

inline void Library::Reset(NativeLibrary native) {
  if (native_ == native)
    return;

  if (native_)
    UnloadNative(native_);

  native_ = native;
}

inline NativeLibrary Library::release() {
  NativeLibrary result = native_;
  native_ = NullNativeLibrary;
  return result;
}

} // namespace stp

#endif // STP_BASE_SYSTEM_LIBRARY_H_
