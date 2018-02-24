// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/TemporaryDirectory.h"

#include "Base/App/Application.h"
#include "Base/Debug/Log.h"
#include "Base/FileSystem/Directory.h"
#include "Base/FileSystem/FilePathWriter.h"
#include "Base/FileSystem/FileSystemException.h"
#include "Base/FileSystem/KnownPaths.h"
#include "Base/Io/StringWriter.h"
#include "Base/Process/NativeProcess.h"
#include "Base/Crypto/CryptoRandom.h"

#if OS(POSIX)
#include "Base/Posix/EintrWrapper.h"
#endif

namespace stp {

/**
 * @class TemporaryDirectory
 * An object representing a temporary / scratch directory that should be cleaned
 * up (recursively) when this object is destroyed.
 */

/**
 * No directory is owned/created initially.
 */
TemporaryDirectory::TemporaryDirectory() {
}

/**
 * Recursively delete path.
 */
TemporaryDirectory::~TemporaryDirectory() {
  if (isValid()) {
    try {
      remove();
    } catch (IoException& exception) {
      LOG(ERROR, "could not delete temp dir in dtor");
    }
  }
}

/**
 * @fn TemporaryDirectory::isValid
 * Returns true if path_ is non-empty.
 */

/**
 * Creates a unique directory in system-wide temporary directory,
 * and takes ownership of it.
 */
void TemporaryDirectory::create() {
  FilePath system_temp_dir = getTempDirPath();
  String prefix = Application::instance().getName();
  createInternal(system_temp_dir, prefix);
}

/**
 * Creates a unique directory under a given path, and takes ownership of it.
 */
void TemporaryDirectory::createUnder(const FilePath& base_path) {
  ASSERT(!isValid());

  // If |base_path| does not exist, create it.
  Directory::createPath(base_path);

  // Create a new, uniquely named directory under |base_path|.
  createInternal(base_path, "temp_dir");
}

/**
 * Takes ownership of directory at @a path, creating it if necessary.
 * Don't call multiple times unless @ref take() has been called first.
 */
void TemporaryDirectory::create(FilePath path) {
  ASSERT(!isValid());

  Directory::createPath(path);

  path_ = move(path);
}

/**
 * Deletes the temporary directory wrapped by this object.
 */
void TemporaryDirectory::remove() {
  ASSERT(isValid());
  FilePath path = move(path_);
  Directory::removeRecursively(path);
}

/**
 * @fn TemporaryDirectory::take
 * Caller takes ownership of the temporary directory so it won't be destroyed
 * when this object goes out of scope.
 */

#if OS(WIN)
void TemporaryDirectory::createInternal(FilePathSpan base_dir, StringSpan prefix) {
  FilePath sub_dir = base_dir;

  CryptoRandom rng;
  for (int count = 0; count < 50; ++count) {
    // Try create a new temporary directory with random generated name.
    // If the one exists, keep trying another path name until we reach some limit.
    FilePathWriter writer(sub_dir);

    writer.ensureSeparator();
    writer << prefix;
    writer << NativeProcess::GetCurrentId();
    writer << '_';
    writer << rng.NextUInt32();

    if (::CreateDirectoryW(toNullTerminated(sub_dir), NULL)) {
      path_ = move(sub_dir);
      return;
    }

    if (::GetLastError() != ERROR_ALREADY_EXISTS)
      throw FileSystemException(getLastWinErrorCode();

    sub_dir.TruncateCharacters(base_dir.size());
  }
  throw FileException() << "unable to create temporary directory with unique name" << base_dir;
}
#elif OS(POSIX)
void TemporaryDirectory::createInternal(FilePathSpan base_dir, StringSpan prefix) {
  String tmpl = String::ConcatArgs(".stp.", prefix, ".XXXXXX");

  FilePath sub_dir;
  sub_dir.ensureCapacity(base_dir.size() + tmpl.size() + 1);
  sub_dir = base_dir;

  FilePathWriter writer(sub_dir);
  writer.ensureSeparator();
  writer << tmpl;

  // this should be OK since mkdtemp just replaces characters in place
  char* buffer = const_cast<char*>(toNullTerminated(sub_dir));
  char* dtemp = ::mkdtemp(buffer);
  if (!dtemp)
    throw FileSystemException(getLastPosixErrorCode());
  ASSERT(dtemp == buffer);

  path_ = move(sub_dir);
}
#endif // OS(*)

} // namespace stp
