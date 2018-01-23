// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_FILEPATHSPAN_H_
#define STP_BASE_FS_FILEPATHSPAN_H_

#include "Base/Compiler/Os.h"
#include "Base/Text/StringSpan.h"

namespace stp {

class FilePath;
class FilePathEnumerator;

#if OS(POSIX)
// On most platforms, native pathnames are char arrays, and the encoding
// may or may not be specified. On Mac OS X, native pathnames are encoded in UTF-8.
typedef char FilePathChar;
# define FILE_PATH_LITERAL(x) x
#elif OS(WIN)
// On Windows, for Unicode-aware applications, native pathnames are wchar_t
// arrays encoded in UTF-16.
typedef wchar_t FilePathChar;
# define FILE_PATH_LITERAL(x) L ## x
#endif

// Windows-style drive letter support and pathname separator characters can be
// enabled and disabled independently, to aid testing.  These #defines are
// here so that the same setting can be used in both the implementation and
// in the unit test.
#define HAVE_FILE_PATH_WITH_DRIVE_LETTER (OS(WIN))

#if OS(POSIX)
constexpr FilePathChar FilePathSeparator = '/';
constexpr FilePathChar FilePathAltSeparator = '/';
#elif OS(WIN)
constexpr FilePathChar FilePathSeparator = L'\\';
constexpr FilePathChar FilePathAltSeparator = L'/';
#endif

constexpr bool IsFilePathSeparator(FilePathChar c) {
  return c == FilePathSeparator || c == FilePathAltSeparator;
}

class BASE_EXPORT FilePathSpan {
 public:
  typedef FilePathChar CharType;

  constexpr FilePathSpan() = default;

  constexpr FilePathSpan(const CharType* data, int size) noexcept : chars_(data, size) {}
  constexpr explicit FilePathSpan(Span<CharType> native) noexcept : chars_(native) {}

  ALWAYS_INLINE constexpr const CharType* data() const { return chars_.data(); }
  ALWAYS_INLINE constexpr int size() const { return chars_.size(); }

  // Returns underlying characters in native encoding.
  // Be very careful on using this. See documentation beforehand.
  ALWAYS_INLINE const Span<CharType>& chars() const { return chars_; }
  ALWAYS_INLINE Span<CharType>& chars() { return chars_; }

  constexpr bool IsEmpty() const { return chars_.IsEmpty(); }

  FilePathSpan GetSlice(int at) const { return FilePathSpan(chars_.GetSlice(at)); }
  FilePathSpan GetSlice(int at, int n) const { return FilePathSpan(chars_.GetSlice(at, n)); }

  void Truncate(int at) { chars_.Truncate(at); }

  FilePathSpan GetRoot() const;
  FilePathSpan GetDirectoryName() const;

  bool CdUp();

  FilePathSpan GetFileName() const;
  FilePathSpan GetFileNameWithoutExtension() const;

  void StripTrailingSeparators();

  int IndexOfSeparator() const;
  int IndexOfSeparator(int begin) const;
  int LastIndexOfSeparator() const;
  int LastIndexOfSeparator(int end) const;

  int IndexOfDriveLetter() const;

  bool HasExtension() const;
  String GetExtension() const;
  bool MatchesExtension(StringSpan extension) const;
  void RemoveExtension();

  bool IsAbsolute() const { return GetRootLength() > 0; }
  bool IsRelative() const { return !IsAbsolute(); }

  FilePathEnumerator Enumerate() const;

  int GetRootLength() const;
  int GetDirectoryNameLength() const;
  int IndexOfExtension() const;
  int CountTrailingSeparators() const;

  friend bool operator<=(const FilePathSpan& l, const FilePathSpan& r) { return l.CompareTo(r) <= 0; }
  friend bool operator>=(const FilePathSpan& l, const FilePathSpan& r) { return l.CompareTo(r) >= 0; }
  friend bool operator< (const FilePathSpan& l, const FilePathSpan& r) { return l.CompareTo(r) <  0; }
  friend bool operator> (const FilePathSpan& l, const FilePathSpan& r) { return l.CompareTo(r) >  0; }

  friend bool operator==(const FilePathSpan& l, const FilePathSpan& r) { return l.EqualsTo(r); }
  friend bool operator!=(const FilePathSpan& l, const FilePathSpan& r) { return !operator==(l, r); }
  friend HashCode Hash(const FilePathSpan& x) { return x.HashImpl(); }
  friend int Compare(const FilePathSpan& l, const FilePathSpan& r) { return l.CompareTo(r); }
  friend TextWriter& operator<<(TextWriter& out, const FilePathSpan& x) {
    x.FormatImpl(out);
    return out;
  }
  friend void Format(TextWriter& out, const FilePathSpan& x, const StringSpan& opts) {
    x.FormatImpl(out);
  }

 private:
  bool EqualsTo(const FilePathSpan& other) const;
  int CompareTo(const FilePathSpan& other) const;
  HashCode HashImpl() const;
  void FormatImpl(TextWriter& out) const;

  Span<CharType> chars_;
};

inline FilePathSpan MakeFilePathSpanFromNullTerminated(const FilePathChar* cstr) {
  return FilePathSpan(MakeSpanFromNullTerminated(cstr));
}

class BASE_EXPORT FilePathEnumerator {
 public:
  explicit FilePathEnumerator(FilePathSpan path);

  FilePathSpan GetCurrent() const { return path_.GetSlice(now_pos_, now_len_); }

  FilePathEnumerator* Next();

  class Iterator {
   public:
    explicit Iterator(FilePathEnumerator* e) : enumerator_(e) {}

    Iterator& operator++() { enumerator_ = enumerator_->Next(); return *this; }
    FilePathSpan operator*() const { return enumerator_->GetCurrent(); }

    bool operator==(const Iterator& o) const { return enumerator_ == o.enumerator_; }
    bool operator!=(const Iterator& o) const { return enumerator_ != o.enumerator_; }

   private:
    FilePathEnumerator* enumerator_;
  };

 private:
  FilePathSpan path_;
  int now_pos_ = 0;
  int now_len_;
};

inline FilePathEnumerator FilePathSpan::Enumerate() const {
  return FilePathEnumerator(*this);
}

} // namespace stp

#endif // STP_BASE_FS_FILEPATHSPAN_H_
