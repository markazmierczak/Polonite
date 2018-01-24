// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/File.h"

#include "Base/Debug/Log.h"
#include "Base/FileSystem/FileSystemException.h"
#include "Base/FileSystem/KnownPaths.h"
#include "Base/Io/FileStream.h"
#include "Base/Text/Format.h"

namespace stp {

void File::GetInfo(const FilePath& path, FileInfo& out) {
  auto error_code = TryGetInfo(path, out);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
}

FilePath File::MakeAbsolutePath(const FilePath& input) {
  FilePath output;
  auto error_code = TryMakeAbsolutePath(input, output);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, input, output);
  return output;
}

void File::Delete(const FilePath& path) {
  auto error_code = TryDelete(path);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, path);
}

void File::Replace(const FilePath& from, const FilePath& to) {
  auto error_code = TryReplace(from, to);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, from, to);
}

Buffer File::ReadAll(const FilePath& path) {
  FileStream file;
  file.Open(path, FileMode::OpenExisting, FileAccess::ReadOnly);

  int64_t length = file.GetLength();

  Buffer output;
  void* dst = output.AppendUninitialized(length);

  file.Read(MutableBufferSpan(dst, length));
  file.Close();
  return output;
}

void File::WriteAll(const FilePath& path, BufferSpan input) {
  FileStream file;
  file.Open(path, FileMode::Create, FileAccess::WriteOnly);
  file.Write(input);
  file.Close();
}

/* FIXME implement
String File::ReadAllText(const FilePath& path, StringSpan codec) {
  FileStream file;
  file.Open(path, FileMode::OpenExisting, FileAccess::ReadOnly);
  int64_t size = file->GetLength();
  if (size < 0)
    return nullopt;

  String buffer;
  char* dst = buffer.AppendUninitialized(size);
  if (file->Read(dst, size) != size)
    return nullopt;

  if (codec.IsEmpty())
    codec = DetectCodec(BufferSpan(dst, size));
  if (codec.IsEmpty())
    return nullopt;

  if (codec == Utf8Codec::Instance.name) {
  }
  // TODO decode
}*/

FilePath File::CreateTemporary() {
  FilePath system_temp_dir = GetTempDirPath();
  return CreateTemporaryIn(system_temp_dir);
}

FilePath File::CreateTemporaryIn(const FilePath& dir) {
  FilePath path;
  auto error_code = TryCreateTemporaryIn(dir, path);
  if (!IsOk(error_code))
    throw FileSystemException(error_code, dir);
  return path;
}

void File::WriteAtomically(const FilePath& path, BufferSpan input) {
  FilePath temp_file_path = CreateTemporaryIn(FilePath(path.GetDirectoryName()));

  FileStream stream;
  stream.Open(temp_file_path, FileMode::OpenExisting, FileAccess::WriteOnly);
  try {
    stream.Write(input);
    stream.Close();
    Replace(temp_file_path, path);
  } catch (...) {
    if (stream.IsOpen())
      stream.Close();
    Delete(temp_file_path);
    throw;
  }
}

} // namespace stp
