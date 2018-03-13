// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_FILEPATH_H_
#define STP_BASE_FS_FILEPATH_H_

#include "Base/Containers/Array.h"
#include "Base/Containers/List.h"
#include "Base/FileSystem/FilePathSpan.h"

namespace stp {

class BASE_EXPORT FilePath {
 public:
  typedef FilePathChar CharType;
  typedef FilePathSpan SpanType;

  FilePath() noexcept = default;
  ~FilePath() = default;

  FilePath(FilePath&& other) noexcept : chars_(move(other.chars_)) {}
  FilePath& operator=(FilePath&& other) noexcept { chars_ = move(other.chars_); return *this; }

  FilePath(const FilePath& other) : chars_(other.chars_) {}
  FilePath& operator=(const FilePath& other) { return operator=(other.toSpan()); }

  explicit FilePath(SpanType path) : chars_(path.chars()) {}
  FilePath& operator=(SpanType path) { chars_ = path.chars(); return *this; }

  explicit FilePath(Span<CharType> chars) : chars_(chars) {}
  FilePath& operator=(Span<CharType> chars) { chars_ = chars; return *this; }

  operator SpanType() const { return toSpan(); }

  FilePath(const CharType* data, int size) : FilePath(SpanType(data, size)) {}

  ALWAYS_INLINE const CharType* data() const { return chars_.data(); }
  ALWAYS_INLINE CharType* data() { return chars_.data(); }
  ALWAYS_INLINE int size() const { return chars_.size(); }
  ALWAYS_INLINE int capacity() const { return chars_.capacity(); }

  ALWAYS_INLINE const List<CharType>& chars() const { return chars_; }
  ALWAYS_INLINE List<CharType>& chars() { return chars_; }

  bool isEmpty() const { return chars_.isEmpty(); }
  void clear() { chars_.clear(); }

  void ensureCapacity(int request);
  void shrinkToFit();

  SpanType slice(int at) const { return toSpan().slice(at); }
  SpanType slice(int at, int n) const { return toSpan().slice(at, n); }

  void truncate(int at) { chars_.truncate(at); }

  SpanType getRoot() const { return toSpan().getRoot(); }
  SpanType getDirectoryName() const { return toSpan().getDirectoryName(); }

  bool cdUp();

  SpanType getFileName() const { return toSpan().getFileName(); }
  SpanType getFileNameWithoutExtension() const { return toSpan().getFileNameWithoutExtension(); }

  void stripTrailingSeparators();

  int indexOfSeparator() const { return toSpan().indexOfSeparator(); }
  int indexOfSeparator(int begin) const { return toSpan().indexOfSeparator(begin); }
  int lastIndexOfSeparator() const { return toSpan().lastIndexOfSeparator(); }
  int lastIndexOfSeparator(int end) const { return toSpan().lastIndexOfSeparator(end); }

  int indexOfDriveLetter() const { return toSpan().indexOfDriveLetter(); }

  StringSpan getExtension() const { return toSpan().getExtension(); }
  bool matchesExtension(StringSpan extension) const { return toSpan().matchesExtension(extension); }
  void removeExtension();
  bool replaceExtension(StringSpan extension);

  bool isAbsolute() const { return toSpan().IsAbsolute(); }
  bool isRelative() const { return toSpan().IsRelative(); }

  FilePathEnumerator enumerate() const { return toSpan().enumerate(); }

  // Normalize all path separators to given type on Windows or do nothing on POSIX systems.
  void normalizeSeparators() { normalizeSeparatorsTo(FilePathSeparator); }
  void normalizeSeparatorsTo(CharType separator);

  // Returns a FilePath object from a path name in UTF encoding.
  static FilePath fromString(StringSpan string);

  void addComponent(SpanType component);
  void addComponentAscii(StringSpan component);

  int getRootLength() const { return toSpan().getRootLength(); }
  int getDirectoryNameLength() const { return toSpan().getDirectoryNameLength(); }
  int indexOfExtension() const { return toSpan().indexOfExtension(); }
  int countTrailingSeparators() const { return toSpan().countTrailingSeparators(); }

  friend bool operator<=(const FilePath& l, const FilePathSpan& r) { return l.toSpan() <= r; }
  friend bool operator>=(const FilePath& l, const FilePathSpan& r) { return l.toSpan() >= r; }
  friend bool operator< (const FilePath& l, const FilePathSpan& r) { return l.toSpan() <  r; }
  friend bool operator> (const FilePath& l, const FilePathSpan& r) { return l.toSpan() >  r; }

  friend void swap(FilePath& l, FilePath& r) noexcept { swap(l.chars_, r.chars_); }
  friend bool operator==(const FilePath& l, FilePathSpan r) { return l.toSpan() == r; }
  friend bool operator!=(const FilePath& l, FilePathSpan r) { return !operator==(l, r); }
  friend HashCode partialHash(const FilePath& x) { return partialHash(x.toSpan()); }
  friend int compare(const FilePath& l, FilePathSpan r) { return compare(l.toSpan(), r); }
  friend TextWriter& operator<<(TextWriter& out, const FilePath& x) { return out << x.toSpan(); }
  friend void format(TextWriter& out, const FilePath& x, const StringSpan& opts) {
    format(out, x.toSpan(), opts);
  }

  friend const FilePathChar* toNullTerminated(const FilePath& x) {
    return toNullTerminated(x.chars_);
  }

  SpanType toSpan() const { return SpanType(chars_); }

 private:
  List<CharType> chars_;
};

namespace detail {
BASE_EXPORT FilePath combineFilePaths(Span<FilePathSpan> components);
}

template<typename... Ts>
inline FilePath combineFilePaths(const Ts&... args) {
  auto array = makeArray<FilePathSpan>(args...);
  return detail::combineFilePaths(array);
}

template<>
struct TIsZeroConstructibleTmpl<FilePath> : TTrue {};
template<>
struct TIsTriviallyRelocatableTmpl<FilePath> : TTrue {};

} // namespace stp

#endif // STP_BASE_FS_FILEPATH_H_
