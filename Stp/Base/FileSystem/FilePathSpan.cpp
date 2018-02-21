// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/FilePathSpan.h"

#include "Base/FileSystem/FilePath.h"
#include "Base/Io/TextWriter.h"
#include "Base/Text/AsciiString.h"
#include "Base/Text/StringAlgo.h"
#include "Base/Text/Wtf.h"

namespace stp {

#if OS(WIN)
static constexpr wchar_t FilePathSeparators[2] = { FilePathSeparator, FilePathAltSeparator };
#endif

/**
 * If this path contains a drive letter specification (can only be true on Windows),
 * returns the position of the character of the drive letter specification,
 * otherwise returns -1.
 */
int FilePathSpan::indexOfDriveLetter() const {
  #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
  // This is dependent on an ASCII-based character set, but that's a
  // reasonable assumption.
  if (size() >= 2 && chars_[1] == L':' && IsAlphaAscii(chars_[0]))
    return 0;
  #endif
  return -1;
}

/** @fn bool FilePathSpan::IsAbsolute() const
 * Returns true if this path contains an absolute path.
 * On Windows, an absolute path begins with either a drive letter specification
 * followed by a separator character, or with two separator characters.
 * On POSIX platforms, an absolute path begins with a separator character.
 */

int FilePathSpan::GetRootLength() const {
  // FIXME for UNC consume whole \\abc\def
  #if OS(WIN)
  int letter = indexOfDriveLetter();
  if (letter >= 0) {
    // Look for a separator right after the drive specification.
    int pos = letter + 2;
    if (size_ > pos && IsFilePathSeparator(chars_[pos]))
      ++pos;
    return pos;
  }
  // Look for a pair of leading separators.
  if (size() > 1 && IsFilePathSeparator(chars_[0]) && IsFilePathSeparator(chars_[1]))
    return 2;
  #else
  // Look for a separator in the first position.
  if (size() > 0 && IsFilePathSeparator(chars_[0])) {
    // Detect alternative root.
    return (size() > 1 && IsFilePathSeparator(chars_[1])) ? 2 : 1;
  }
  #endif
  return 0;
}

FilePathSpan FilePathSpan::GetRoot() const {
  return FilePathSpan(data(), GetRootLength());
}

int FilePathSpan::GetDirectoryNameLength() const {
  int root_len = GetRootLength();
  int last_separator = lastIndexOfSeparator();

  int pos = last_separator;
  for (; pos > root_len; --pos) {
    if (!IsFilePathSeparator(chars_[pos - 1]))
      break;
  }
  return pos <= root_len ? root_len : pos;
}

/**
 * Return a path corresponding to the directory containing the path
 * named by this object, stripping away the file component.
 * If this object refers to root directory, returns root directory.
 * If this object only contains one component returns empty string.
 */
FilePathSpan FilePathSpan::GetDirectoryName() const {
  return FilePathSpan(chars_.data(), GetDirectoryNameLength());
}

/**
 * Returns a path corresponding to the last path component of this
 * object, either a file or a directory. If this object refers to
 * the root directory, returns empty path.
 */
FilePathSpan FilePathSpan::GetFileName() const {
  int root_len = GetRootLength();

  // Keep everything after the final separator.
  int last_separator = lastIndexOfSeparator();
  if (last_separator < 0)
    last_separator = root_len - 1;
  if (last_separator < 0)
    return *this;

  return FilePathSpan(chars_.getSlice(last_separator + 1));
}

FilePathSpan FilePathSpan::GetFileNameWithoutExtension() const {
  FilePathSpan filename = GetFileName();
  filename.RemoveExtension();
  return filename;
}

int FilePathSpan::CountTrailingSeparators() const {
  int root_length = GetRootLength();
  int count = 0;
  int length = size();
  for (; count < length - root_length; ++count) {
    if (!IsFilePathSeparator(chars_[length - (count + 1)]))
      break;
  }
  return count;
}

void FilePathSpan::StripTrailingSeparators() {
  chars_.removeSuffix(CountTrailingSeparators());
}

/**
 * Same as GetDirectoryName() but in-place.
 * Return true if path is non-empty after operation.
 */
bool FilePathSpan::CdUp() {
  int root_length = GetRootLength();
  chars_.truncate(GetDirectoryNameLength());
  return chars_.size() != root_length;
}

int FilePathSpan::indexOfSeparator() const {
  #if OS(WIN)
  return chars_.indexOfAny(FilePathSeparators);
  #elif OS(POSIX)
  return chars_.indexOf(FilePathSeparator);
  #endif
}

int FilePathSpan::lastIndexOfSeparator() const {
  #if OS(WIN)
  return chars_.lastIndexOfAny(FilePathSeparators);
  #elif OS(POSIX)
  return chars_.lastIndexOf(FilePathSeparator);
  #endif
}

int FilePathSpan::indexOfSeparator(int start) const {
  auto slice = getSlice(start);
  int found = slice.indexOfSeparator();
  return found >= 0 ? (found + start) : found;
}

int FilePathSpan::lastIndexOfSeparator(int start) const {
  auto slice = getSlice(0, start + 1);
  return slice.lastIndexOfSeparator();
}

int FilePathSpan::indexOfExtension() const {
  // Must be "> 0" due how extensions work, there must be something before dot.
  auto* this_data = data();
  int this_size = size();

  for (int i = this_size - 1; i > 0; --i) {
    auto c = this_data[i];
    if (c == '.') {
      char before = this_data[i - 1];
      if (i == this_size - 1) {
        if (IsFilePathSeparator(before))
          return -1; // dot
        if (before == '.') {
          if (this_size == 2 || IsFilePathSeparator(this_data[i - 2]))
            return -1; // dot dot
        }
        return i;
      }
      if (IsFilePathSeparator(before))
        return -1; // hidden file, e.g. ".git"
      return i;
    }
    if (IsFilePathSeparator(c))
      return -1;
    if (!isAscii(c))
      return -1;
  }
  return -1;
}

bool FilePathSpan::HasExtension() const {
  return indexOfExtension() >= 0;
}

/**
 * Returns `.jpg` for path `C:\pics\jojo.jpg`, or an empty string if
 * the file has no extension. If non-empty, GetExtension() will always start
 * with precisely one dot.
 */
String FilePathSpan::GetExtension() const {
  int pos = indexOfExtension();

  String result;
  if (pos < 0)
    return result;

  // Extract extension (including dot).
  int ext_size = size() - pos;
  char* dst = result.appendUninitialized(ext_size);
  // We already know the extension is ASCII encoded, see FindExtension().
  for (int i = 0; i < ext_size; ++i)
    dst[i] = static_cast<char>(chars_[pos + i]);
  return result;
}

/**
 * Returns true if the file path matches the specified extension.
 * The test is case insensitive.
 */
bool FilePathSpan::MatchesExtension(StringSpan extension) const {
  ASSERT(isAscii(extension));

  int pos = indexOfExtension();

  bool no_extension = pos < 0;
  bool expect_no_extension = extension.isEmpty();
  if (no_extension || expect_no_extension)
    return no_extension == expect_no_extension;

  // skip dot
  ++pos;
  if (extension.getFirst() == '.')
    extension.removePrefix(1);

  // compare lengths
  int ext_size = size() - pos;
  if (ext_size != extension.size())
    return false;

  // inline this loop since extension is usually short
  for (int i = 0; i < ext_size; ++i) {
    if (toLowerAscii(extension[i]) != toLowerAscii(chars_[pos + i]))
      return false;
  }
  return true;
}

/**
 * Trims extension from this string, e.g."
 * "C:\pics\jojo.jpg" -> "C:\pics\jojo"
 */
void FilePathSpan::RemoveExtension() {
  int pos = indexOfExtension();
  if (pos >= 0)
    chars_.truncate(pos);
}

void FilePathSpan::formatImpl(TextWriter& out) const {
  #if HAVE_UTF8_NATIVE_VALIDATION
  out.Write(chars_);
  #else
  WriteWtf(out, chars_);
  #endif
}

FilePathEnumerator::FilePathEnumerator(FilePathSpan path)
    : path_(path) {
  int root_size = path_.GetRootLength();
  if (root_size > 0)
    now_len_ = root_size;
  else
    now_len_ = max(0, path_.indexOfSeparator());
}

FilePathEnumerator* FilePathEnumerator::Next() {
  int path_size = path_.size();
  auto* path_chars = path_.data();
  // skip old component and separators
  int old_end = now_pos_ + now_len_;
  int pos = old_end;
  for (; pos < path_size; ++pos) {
    if (!IsFilePathSeparator(path_chars[pos]))
      break;
  }
  // check if we are done
  now_pos_ = pos;
  if (pos == path_size) {
    if (pos == old_end)
      // had separator at the end ?
      return nullptr;

    now_len_ = 0;
    return this;
  }
  // find next component after ours
  int next_sep = path_.indexOfSeparator(pos);
  now_len_ = (next_sep >= 0 ? next_sep : path_size) - pos;
  return this;
}

} // namespace stp
