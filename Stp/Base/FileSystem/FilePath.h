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

  FilePath(FilePath&& other) noexcept : chars_(Move(other.chars_)) {}
  FilePath& operator=(FilePath&& other) noexcept { chars_ = Move(other.chars_); return *this; }

  FilePath(const FilePath& other) : chars_(other.chars_) {}
  FilePath& operator=(const FilePath& other) { return operator=(other.ToSpan()); }

  explicit FilePath(SpanType path) : chars_(path.chars()) {}
  FilePath& operator=(SpanType path) { chars_ = path.chars(); return *this; }

  explicit FilePath(Span<CharType> chars) : chars_(chars) {}
  FilePath& operator=(Span<CharType> chars) { chars_ = chars; return *this; }

  operator SpanType() const { return ToSpan(); }

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

  SpanType GetSlice(int at) const { return ToSpan().GetSlice(at); }
  SpanType GetSlice(int at, int n) const { return ToSpan().GetSlice(at, n); }

  void Truncate(int at) { chars_.Truncate(at); }

  SpanType GetRoot() const { return ToSpan().GetRoot(); }
  SpanType GetDirectoryName() const { return ToSpan().GetDirectoryName(); }

  bool CdUp();

  SpanType GetFileName() const { return ToSpan().GetFileName(); }
  SpanType GetFileNameWithoutExtension() const { return ToSpan().GetFileNameWithoutExtension(); }

  void StripTrailingSeparators();

  int IndexOfSeparator() const { return ToSpan().IndexOfSeparator(); }
  int IndexOfSeparator(int begin) const { return ToSpan().IndexOfSeparator(begin); }
  int LastIndexOfSeparator() const { return ToSpan().LastIndexOfSeparator(); }
  int LastIndexOfSeparator(int end) const { return ToSpan().LastIndexOfSeparator(end); }

  int IndexOfDriveLetter() const { return ToSpan().IndexOfDriveLetter(); }

  StringSpan GetExtension() const { return ToSpan().GetExtension(); }
  bool MatchesExtension(StringSpan extension) const { return ToSpan().MatchesExtension(extension); }
  void RemoveExtension();
  bool ReplaceExtension(StringSpan extension);

  bool IsAbsolute() const { return ToSpan().IsAbsolute(); }
  bool IsRelative() const { return ToSpan().IsRelative(); }

  FilePathEnumerator Enumerate() const { return ToSpan().Enumerate(); }

  // Normalize all path separators to given type on Windows or do nothing on POSIX systems.
  void NormalizeSeparators() { NormalizeSeparatorsTo(FilePathSeparator); }
  void NormalizeSeparatorsTo(CharType separator);

  // Returns a FilePath object from a path name in UTF encoding.
  static FilePath FromString(StringSpan string);
  static FilePath FromString(String16Span string);
  #if OS(WIN)
  static FilePath FromString(WStringSpan string);
  #endif

  void Add(SpanType component);
  void AddAscii(StringSpan component);

  void AppendChars(Span<CharType> chars);
  CharType* AppendCharsUninitialized(int n);

  int GetRootLength() const { return ToSpan().GetRootLength(); }
  int GetDirectoryNameLength() const { return ToSpan().GetDirectoryNameLength(); }
  int IndexOfExtension() const { return ToSpan().IndexOfExtension(); }
  int CountTrailingSeparators() const { return ToSpan().CountTrailingSeparators(); }

  friend bool operator<=(const FilePath& l, const FilePathSpan& r) { return l.ToSpan() <= r; }
  friend bool operator>=(const FilePath& l, const FilePathSpan& r) { return l.ToSpan() >= r; }
  friend bool operator< (const FilePath& l, const FilePathSpan& r) { return l.ToSpan() <  r; }
  friend bool operator> (const FilePath& l, const FilePathSpan& r) { return l.ToSpan() >  r; }

  friend void Swap(FilePath& l, FilePath& r) noexcept { Swap(l.chars_, r.chars_); }
  friend bool operator==(const FilePath& l, FilePathSpan r) { return l.ToSpan() == r; }
  friend bool operator!=(const FilePath& l, FilePathSpan r) { return !operator==(l, r); }
  friend HashCode Hash(const FilePath& x) { return Hash(x.ToSpan()); }
  friend int Compare(const FilePath& l, FilePathSpan r) { return Compare(l.ToSpan(), r); }
  friend TextWriter& operator<<(TextWriter& out, const FilePath& x) { return out << x.ToSpan(); }
  friend void Format(TextWriter& out, const FilePath& x, const StringSpan& opts) {
    Format(out, x.ToSpan(), opts);
  }

  friend const FilePathChar* ToNullTerminated(const FilePath& x) {
    return ToNullTerminated(x.chars_);
  }

  friend SpanType MakeSpan(const FilePath& x) { return x.ToSpan(); }

 private:
  List<CharType> chars_;

  SpanType ToSpan() const { return SpanType(chars_); }
};

namespace detail {
BASE_EXPORT FilePath CombineFilePaths(Span<FilePathSpan> components);
}

template<typename... Ts>
inline FilePath CombineFilePaths(const Ts&... args) {
  auto array = MakeArray<FilePathSpan>(args...);
  return detail::CombineFilePaths(array);
}

template<>
struct TIsZeroConstructibleTmpl<FilePath> : TTrue {};
template<>
struct TIsTriviallyRelocatableTmpl<FilePath> : TTrue {};

} // namespace stp

#endif // STP_BASE_FS_FILEPATH_H_
