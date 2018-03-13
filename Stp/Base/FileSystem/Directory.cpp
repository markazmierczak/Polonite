// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/Directory.h"

#include "Base/Containers/InlineList.h"
#include "Base/FileSystem/File.h"
#include "Base/FileSystem/FileInfo.h"
#include "Base/FileSystem/FileSystemException.h"
#include "Base/FileSystem/RecursiveDirectoryEnumerator.h"

namespace stp {

void Directory::create(const FilePath& path) {
  auto error_code = tryCreate(path);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
}

void Directory::createPath(const FilePath& path) {
  auto error_code = tryCreatePath(path);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
}

void Directory::removeEmpty(const FilePath& path) {
  auto error_code = tryRemoveEmpty(path);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
}

SystemErrorCode Directory::tryCreatePath(const FilePath& path) {
  SystemErrorCode error_code = tryCreate(path);
  if (isOk(error_code))
    return error_code;

  // Slow path: Collect a list of all parent directories.

  // Make a copy to be able to truncate path at certain positions
  // and create sub-paths.
  auto copy = FilePath(path);

  // A list of sub-paths. Each value is a length from beginning of original
  // path to the end of sub-path.
  InlineList<int, 16> subpaths;

  FilePathSpan subpath = path;
  do {
    subpaths.add(subpath.size());
  } while (subpath.cdUp());

  int path_length = path.size();
  FilePathChar* copyd = copy.data();

  // Iterate through the parents and create the missing ones.
  for (int i = subpaths.size() - 1; i >= 0; --i) {
    int offset = subpaths[i];
    FilePathChar char_copy = copyd[offset];
    copy.chars().truncate(offset);

    error_code = tryCreate(copy);

    *copy.chars().appendUninitialized(path_length - offset) = char_copy;

    if (!isOk(error_code))
      break;
  }
  return error_code;
}

void Directory::removeRecursively(const FilePath& path) {
  List<FilePath> directories;
  directories.add(FilePath(path));

  RecursiveDirectoryEnumerator enumerator;
  while (!directories.isEmpty()) {
    enumerator.open(directories.last());

    bool has_nested = false;
    while (enumerator.moveNext()) {
      FilePath full_path = enumerator.getEntryFullPath();
      if (enumerator.base().isDirectory()) {
        directories.add(move(full_path));
        has_nested = true;
      } else {
        File::remove(full_path);
      }
    }
    if (!has_nested) {
      removeEmpty(directories.last());
      directories.removeLast();
    }
    enumerator.close();
  }
}

uint64_t Directory::computeSize(const FilePath& path) {
  uint64_t result = 0;

  #if OS(POSIX)
  FileInfo file_info;
  #endif

  RecursiveDirectoryEnumerator enumerator;
  enumerator.open(path);
  while (enumerator.moveNext()) {
    #if OS(WIN)
    result += enumerator.getSize();
    #elif OS(POSIX)
    File::getInfo(enumerator.getEntryFullPath(), file_info);
    result += file_info.getSize();
    #endif
  }
  enumerator.close();
  return result;
}

Directory::DriveSpaceInfo Directory::getDriveSpaceInfo(const FilePath& path) {
  DriveSpaceInfo info;
  auto error_code = tryGetDriveSpaceInfo(path, info);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
  return info;
}

} // namespace stp
