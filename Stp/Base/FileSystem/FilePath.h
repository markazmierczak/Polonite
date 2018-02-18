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

  bool IsEmpty() const { return chars_.IsEmpty(); }
  void Clear() { chars_.Clear(); }

  void EnsureCapacity(int request);
  void ShrinkToFit();

  SpanType getSlice(int at) const { return toSpan().getSlice(at); }
  SpanType getSlice(int at, int n) const { return toSpan().getSlice(at, n); }

  void Truncate(int at) { chars_.Truncate(at); }

  SpanType GetRoot() const { return toSpan().GetRoot(); }
  SpanType GetDirectoryName() const { return toSpan().GetDirectoryName(); }

  bool CdUp();

  SpanType GetFileName() const { return toSpan().GetFileName(); }
  SpanType GetFileNameWithoutExtension() const { return toSpan().GetFileNameWithoutExtension(); }

  void StripTrailingSeparators();

  int IndexOfSeparator() const { return toSpan().IndexOfSeparator(); }
  int IndexOfSeparator(int begin) const { return toSpan().IndexOfSeparator(begin); }
  int LastIndexOfSeparator() const { return toSpan().LastIndexOfSeparator(); }
  int LastIndexOfSeparator(int end) const { return toSpan().LastIndexOfSeparator(end); }

  int IndexOfDriveLetter() const { return toSpan().IndexOfDriveLetter(); }

  StringSpan GetExtension() const { return toSpan().GetExtension(); }
  bool MatchesExtension(StringSpan extension) const { return toSpan().MatchesExtension(extension); }
  void RemoveExtension();
  bool ReplaceExtension(StringSpan extension);

  bool IsAbsolute() const { return toSpan().IsAbsolute(); }
  bool IsRelative() const { return toSpan().IsRelative(); }

  FilePathEnumerator Enumerate() const { return toSpan().Enumerate(); }

  // Normalize all path separators to given type on Windows or do nothing on POSIX systems.
  void NormalizeSeparators() { NormalizeSeparatorsTo(FilePathSeparator); }
  void NormalizeSeparatorsTo(CharType separator);

  // Returns a FilePath object from a path name in UTF encoding.
  static FilePath FromString(StringSpan string);

  void AddComponent(SpanType component);
  void AddComponentAscii(StringSpan component);

  int GetRootLength() const { return toSpan().GetRootLength(); }
  int GetDirectoryNameLength() const { return toSpan().GetDirectoryNameLength(); }
  int IndexOfExtension() const { return toSpan().IndexOfExtension(); }
  int CountTrailingSeparators() const { return toSpan().CountTrailingSeparators(); }

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

  friend const FilePathChar* ToNullTerminated(const FilePath& x) {
    return ToNullTerminated(x.chars_);
  }

  friend SpanType makeSpan(const FilePath& x) { return x.toSpan(); }

 private:
  List<CharType> chars_;

  SpanType toSpan() const { return SpanType(chars_); }
};

namespace detail {
BASE_EXPORT FilePath CombineFilePaths(Span<FilePathSpan> components);
}

template<typename... Ts>
inline FilePath CombineFilePaths(const Ts&... args) {
  auto array = makeArray<FilePathSpan>(args...);
  return detail::CombineFilePaths(array);
}

template<>
struct TIsZeroConstructibleTmpl<FilePath> : TTrue {};
template<>
struct TIsTriviallyRelocatableTmpl<FilePath> : TTrue {};

} // namespace stp

#endif // STP_BASE_FS_FILEPATH_H_
