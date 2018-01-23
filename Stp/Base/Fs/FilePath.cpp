// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Fs/FilePath.h"

#include "Base/Containers/ContiguousAlgo.h"
#include "Base/Math/Alignment.h"
#include "Base/Text/AsciiString.h"
#include "Base/Text/Wtf.h"

#if OS(WIN)
# include <windows.h>
#elif OS(DARWIN)
# include <CoreFoundation/CoreFoundation.h>
#endif

namespace stp {

/** @class FilePath
 * An abstraction to isolate users from the differences between native
 * pathnames on different platforms.
 */

void FilePath::EnsureCapacity(int request) {
  chars_.EnsureCapacity(request);
}

void FilePath::ShrinkToFit() {
  chars_.ShrinkToFit();
}

void FilePath::AppendChars(Span<CharType> chars) {
  chars_.Append(chars);
}

FilePathChar* FilePath::AppendCharsUninitialized(int n) {
  return chars_.AppendUninitialized(n);
}

bool FilePath::CdUp() {
  Truncate(GetDirectoryNameLength());
  return !IsEmpty();
}

void FilePath::RemoveExtension() {
  int pos = IndexOfExtension();
  if (pos >= 0)
    Truncate(pos);
}

// Returns true if path is "", ".", or "..".
static bool IsEmptyOrSpecialCase(Span<FilePathChar> path) {
  if (path.size() > 2)
    return false;
  for (int i = 0 ; i < path.size(); ++i) {
    if (path[i] != '.')
      return false;
  }
  return true;
}

/**
 * Replaces the extension of the path with given @a extension.
 * If the path does not have an extension, the @a extension is added.
 * If given @a extension is empty, then the extension is removed from this path.
 *
 * @param extension
 *   A file extension as ASCII string.
 *   If empty, the previous extension is simply removed.
 *   Whether it starts with a dot is not important (both cases are handled identically).
 * @return false if this path is empty or its filename is dot or dot-dot, true otherwise.
 */
bool FilePath::ReplaceExtension(StringSpan extension) {
  int pos = IndexOfExtension();
  if (pos < 0) {
    FilePathSpan filename = GetFileName();
    if (IsEmptyOrSpecialCase(filename.chars()))
      return false;
  } else {
    Truncate(pos);
  }
  if (!extension.IsEmpty()) {
    if (extension[0] != '.')
      chars_.Add('.');
    AppendChars(extension);
  }
  return true;
}

void FilePath::StripTrailingSeparators() {
  chars_.RemoveSuffix(CountTrailingSeparators());
}

void FilePath::NormalizeSeparatorsTo(CharType separator) {
  ASSERT(IsFilePathSeparator(separator));
  #if OS(WIN)
  Replace(chars_, separator == Separator ? AltSeparator : Separator, separator);
  #endif
}

FilePath FilePath::FromString(StringSpan string) {
  #if OS(POSIX)
  return FilePath(string.data(), string.size());
  #else
  return FromStringTmpl(string);
  #endif
}

#if OS(WIN)
FilePath FilePath::FromString(WStringSpan string) {
  return FilePath(string.data(), string.size());
}
#endif

void FilePath::Add(FilePathSpan component) {
  ASSERT(!component.IsAbsolute());
  ASSERT(!chars_.IsSourceOf(component.chars()));

  bool need_separator = false;
  if (!IsEmpty())
    need_separator = !IsFilePathSeparator(chars_.GetLast());

  int length = component.size();
  CharType* dst = AppendCharsUninitialized(length + (need_separator ? 1 : 0));

  if (need_separator)
    *dst++ = FilePathSeparator;
  UninitializedCopy(dst, component.data(), length);
}

void FilePath::AddAscii(StringSpan component) {
  ASSERT(IsAscii(component));

  bool need_separator = false;
  if (!IsEmpty())
    need_separator = !IsFilePathSeparator(chars_.GetLast());

  int length = component.size();
  auto* src = component.data();
  CharType* dst = AppendCharsUninitialized(length + (need_separator ? 1 : 0));

  if (need_separator)
    *dst++ = FilePathSeparator;
  for (int i = 0; i < length; ++i)
    dst[i] = char_cast<CharType>(src[i]);
}

namespace detail {

FilePath CombineFilePaths(Span<FilePathSpan> components) {
  int n = components.size();
  FilePath result;
  result.EnsureCapacity(Accumulate(components, n, [](int init, const FilePathSpan& component) {
    return init + component.size();
  }));
  for (int i = 0; i < n; ++i)
    result.Add(components[i]);
  return result;
}

} // namespace detail

} // namespace stp
