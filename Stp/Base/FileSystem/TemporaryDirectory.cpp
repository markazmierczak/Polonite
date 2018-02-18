// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/FileSystem/TemporaryDirectory.h"

#include "Base/App/Application.h"
#include "Base/Debug/Log.h"
#include "Base/FileSystem/Directory.h"
#include "Base/FileSystem/FilePathWriter.h"
#include "Base/FileSystem/KnownPaths.h"
#include "Base/Io/StringWriter.h"
#include "Base/Process/NativeProcess.h"
#include "Base/Crypto/CryptoRandom.h"
#include "Base/Text/FormatMany.h"

#if OS(POSIX)
#include "Base/Posix/EintrWrapper.h"
#endif

namespace stp {

TemporaryDirectory::TemporaryDirectory() {
}

TemporaryDirectory::~TemporaryDirectory() {
  if (IsValid()) {
    try {
      Remove();
    } catch (IoException& exception) {
      LOG(ERROR, "could not delete temp dir in dtor");
    }
  }
}

void TemporaryDirectory::Create() {
  FilePath system_temp_dir = GetTempDirPath();
  String prefix = Application::instance().getName();
  CreateInternal(system_temp_dir, prefix);
}

void TemporaryDirectory::CreateUnder(const FilePath& base_path) {
  ASSERT(!IsValid());

  // If |base_path| does not exist, create it.
  Directory::CreatePath(base_path);

  // Create a new, uniquely named directory under |base_path|.
  CreateInternal(base_path, "temp_dir");
}

void TemporaryDirectory::Create(FilePath path) {
  ASSERT(!IsValid());

  Directory::CreatePath(path);

  path_ = move(path);
}

void TemporaryDirectory::Remove() {
  ASSERT(IsValid());
  FilePath path = move(path_);
  Directory::RemoveRecursively(path);
}

#if OS(WIN)
void TemporaryDirectory::CreateInternal(FilePathSpan base_dir, StringSpan prefix) {
  FilePath sub_dir = base_dir;

  CryptoRandom rng;
  for (int count = 0; count < 50; ++count) {
    // Try create a new temporary directory with random generated name.
    // If the one exists, keep trying another path name until we reach some limit.
    FilePathWriter writer(sub_dir);

    writer.EnsureSeparator();
    writer << prefix;
    writer << NativeProcess::GetCurrentId();
    writer << '_';
    writer << rng.NextUInt32();

    if (::CreateDirectoryW(ToNullTerminated(sub_dir), NULL)) {
      path_ = move(sub_dir);
      return;
    }

    if (::GetLastError() != ERROR_ALREADY_EXISTS)
      throw FileSystemException(GetLastWinErrorCode();

    sub_dir.TruncateCharacters(base_dir.size());
  }
  throw FileException() << "unable to create temporary directory with unique name" << base_dir;
}
#elif OS(POSIX)
void TemporaryDirectory::CreateInternal(FilePathSpan base_dir, StringSpan prefix) {
  String tmpl = String::ConcatArgs(".stp.", prefix, ".XXXXXX");

  FilePath sub_dir;
  sub_dir.EnsureCapacity(base_dir.size() + tmpl.size() + 1);
  sub_dir = base_dir;

  FilePathWriter writer(sub_dir);
  writer.EnsureSeparator();
  writer << tmpl;

  // this should be OK since mkdtemp just replaces characters in place
  char* buffer = const_cast<char*>(ToNullTerminated(sub_dir));
  char* dtemp = ::mkdtemp(buffer);
  if (!dtemp)
    throw FileSystemException(GetLastPosixErrorCode());
  ASSERT(dtemp == buffer);

  path_ = move(sub_dir);
}
#endif // OS(*)

} // namespace stp
