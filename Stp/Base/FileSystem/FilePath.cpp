// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/FilePath.h"

#include "Base/Containers/SpanAlgo.h"
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

void FilePath::ensureCapacity(int request) {
  chars_.ensureCapacity(request);
}

void FilePath::shrinkToFit() {
  chars_.shrinkToFit();
}

bool FilePath::cdUp() {
  truncate(getDirectoryNameLength());
  return !isEmpty();
}

void FilePath::removeExtension() {
  int pos = indexOfExtension();
  if (pos >= 0)
    truncate(pos);
}

// Returns true if path is "", ".", or "..".
static bool isEmptyOrSpecialCase(Span<FilePathChar> path) {
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
bool FilePath::replaceExtension(StringSpan extension) {
  int pos = indexOfExtension();
  if (pos < 0) {
    FilePathSpan filename = getFileName();
    if (isEmptyOrSpecialCase(filename.chars()))
      return false;
  } else {
    truncate(pos);
  }
  if (!extension.isEmpty()) {
    if (extension[0] != '.')
      chars_.add('.');
    chars_.append(extension);
  }
  return true;
}

void FilePath::stripTrailingSeparators() {
  chars_.removeSuffix(countTrailingSeparators());
}

void FilePath::normalizeSeparatorsTo(CharType separator) {
  ASSERT(isFilePathSeparator(separator));
  #if OS(WIN)
  replace(chars_, separator == Separator ? AltSeparator : Separator, separator);
  #endif
}

FilePath FilePath::fromString(StringSpan string) {
  #if OS(POSIX)
  return FilePath(string.data(), string.size());
  #elif OS(WIN)
  return fromStringTmpl(string);
  #endif
}

void FilePath::addComponent(FilePathSpan component) {
  ASSERT(!component.IsAbsolute());
  ASSERT(!chars_.isSourceOf(component.chars()));

  bool need_separator = false;
  if (!isEmpty())
    need_separator = !isFilePathSeparator(chars_.last());

  int length = component.size();
  CharType* dst = chars_.appendUninitialized(length + (need_separator ? 1 : 0));

  if (need_separator)
    *dst++ = FilePathSeparator;
  uninitializedCopy(dst, component.data(), length);
}

void FilePath::addComponentAscii(StringSpan component) {
  ASSERT(isAscii(component));

  bool need_separator = false;
  if (!isEmpty())
    need_separator = !isFilePathSeparator(chars_.last());

  int length = component.size();
  auto* src = component.data();
  CharType* dst = chars_.appendUninitialized(length + (need_separator ? 1 : 0));

  if (need_separator)
    *dst++ = FilePathSeparator;
  for (int i = 0; i < length; ++i)
    dst[i] = charCast<CharType>(src[i]);
}

namespace detail {

FilePath combineFilePaths(Span<FilePathSpan> components) {
  int n = components.size();
  FilePath result;
  result.ensureCapacity(accumulateSpan(components, n, [](int init, const FilePathSpan& component) {
    return init + component.size();
  }));
  for (int i = 0; i < n; ++i)
    result.addComponent(components[i]);
  return result;
}

} // namespace detail

} // namespace stp
