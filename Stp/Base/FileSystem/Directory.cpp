// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/Directory.h"

#include "Base/Containers/InlineList.h"
#include "Base/FileSystem/File.h"
#include "Base/FileSystem/FileInfo.h"
#include "Base/FileSystem/FileSystemException.h"
#include "Base/FileSystem/RecursiveDirectoryEnumerator.h"

namespace stp {

void Directory::Create(const FilePath& path) {
  auto error_code = TryCreate(path);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
}

void Directory::CreatePath(const FilePath& path) {
  auto error_code = TryCreatePath(path);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
}

void Directory::RemoveEmpty(const FilePath& path) {
  auto error_code = TryRemoveEmpty(path);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
}

SystemErrorCode Directory::TryCreatePath(const FilePath& path) {
  SystemErrorCode error_code = TryCreate(path);
  if (IsOk(error_code))
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
    subpaths.Add(subpath.size());
  } while (subpath.CdUp());

  int path_length = path.size();
  FilePathChar* copyd = copy.data();

  // Iterate through the parents and create the missing ones.
  for (int i = subpaths.size() - 1; i >= 0; --i) {
    int offset = subpaths[i];
    FilePathChar char_copy = copyd[offset];
    copy.chars().Truncate(offset);

    error_code = TryCreate(copy);

    *copy.chars().AppendUninitialized(path_length - offset) = char_copy;

    if (!IsOk(error_code))
      break;
  }
  return error_code;
}

void Directory::RemoveRecursively(const FilePath& path) {
  List<FilePath> directories;
  directories.Add(FilePath(path));

  RecursiveDirectoryEnumerator enumerator;
  while (!directories.IsEmpty()) {
    enumerator.Open(directories.GetLast());

    bool has_nested = false;
    while (enumerator.MoveNext()) {
      FilePath full_path = enumerator.GetEntryFullPath();
      if (enumerator.base().IsDirectory()) {
        directories.Add(Move(full_path));
        has_nested = true;
      } else {
        File::Delete(full_path);
      }
    }
    if (!has_nested) {
      RemoveEmpty(directories.GetLast());
      directories.RemoveLast();
    }
    enumerator.Close();
  }
}

uint64_t Directory::ComputeSize(const FilePath& path) {
  uint64_t result = 0;

  #if OS(POSIX)
  FileInfo file_info;
  #endif

  RecursiveDirectoryEnumerator enumerator;
  enumerator.Open(path);
  while (enumerator.MoveNext()) {
    #if OS(WIN)
    result += enumerator.GetSize();
    #elif OS(POSIX)
    File::GetInfo(enumerator.GetEntryFullPath(), file_info);
    result += file_info.GetSize();
    #endif
  }
  enumerator.Close();
  return result;
}

Directory::DriveSpaceInfo Directory::GetDriveSpaceInfo(const FilePath& path) {
  DriveSpaceInfo info;
  auto error_code = TryGetDriveSpaceInfo(path, info);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
  return info;
}

} // namespace stp
