// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/File.h"

#include "Base/Debug/Log.h"
#include "Base/FileSystem/FileSystemException.h"
#include "Base/FileSystem/KnownPaths.h"
#include "Base/Io/FileStream.h"

namespace stp {

void File::getInfo(const FilePath& path, FileInfo& out) {
  auto error_code = tryGetInfo(path, out);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
}

FilePath File::makeAbsolutePath(const FilePath& input) {
  FilePath output;
  auto error_code = tryMakeAbsolutePath(input, output);
  if (!isOk(error_code))
    throw FileSystemException(error_code, input, output);
  return output;
}

void File::remove(const FilePath& path) {
  auto error_code = tryRemove(path);
  if (!isOk(error_code))
    throw FileSystemException(error_code, path);
}

void File::replace(const FilePath& from, const FilePath& to) {
  auto error_code = tryReplace(from, to);
  if (!isOk(error_code))
    throw FileSystemException(error_code, from, to);
}

Buffer File::readAll(const FilePath& path) {
  FileStream file;
  file.open(path, FileMode::OpenExisting, FileAccess::ReadOnly);

  int64_t length = file.getLength();

  Buffer output;
  void* dst = output.appendUninitialized(length);

  file.read(MutableBufferSpan(dst, length));
  file.close();
  return output;
}

void File::writeAll(const FilePath& path, BufferSpan input) {
  FileStream file;
  file.open(path, FileMode::Create, FileAccess::WriteOnly);
  file.write(input);
  file.close();
}

/* FIXME implement
String File::readAllText(const FilePath& path, StringSpan codec) {
  FileStream file;
  file.open(path, FileMode::OpenExisting, FileAccess::ReadOnly);
  int64_t size = file->getLength();
  if (size < 0)
    return nullopt;

  String buffer;
  char* dst = buffer.appendUninitialized(size);
  if (file->Read(dst, size) != size)
    return nullopt;

  if (codec.isEmpty())
    codec = DetectCodec(BufferSpan(dst, size));
  if (codec.isEmpty())
    return nullopt;

  if (codec == Utf8Codec::Instance.name) {
  }
  // TODO decode
}*/

FilePath File::createTemporary() {
  FilePath system_temp_dir = getTempDirPath();
  return createTemporaryIn(system_temp_dir);
}

FilePath File::createTemporaryIn(const FilePath& dir) {
  FilePath path;
  auto error_code = tryCreateTemporaryIn(dir, path);
  if (!isOk(error_code))
    throw FileSystemException(error_code, dir);
  return path;
}

void File::writeAtomically(const FilePath& path, BufferSpan input) {
  FilePath temp_file_path = createTemporaryIn(FilePath(path.getDirectoryName()));

  FileStream stream;
  stream.open(temp_file_path, FileMode::OpenExisting, FileAccess::WriteOnly);
  try {
    stream.write(input);
    stream.close();
    replace(temp_file_path, path);
  } catch (...) {
    if (stream.isOpen())
      stream.close();
    ignoreResult(tryRemove(temp_file_path));
    throw;
  }
}

} // namespace stp
