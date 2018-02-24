// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_FILEPATHSPAN_H_
#define STP_BASE_FS_FILEPATHSPAN_H_

#include "Base/Compiler/Os.h"
#include "Base/Containers/Span.h"

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

constexpr bool isFilePathSeparator(FilePathChar c) {
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

  constexpr bool isEmpty() const { return chars_.isEmpty(); }

  FilePathSpan getSlice(int at) const { return FilePathSpan(chars_.getSlice(at)); }
  FilePathSpan getSlice(int at, int n) const { return FilePathSpan(chars_.getSlice(at, n)); }

  void truncate(int at) { chars_.truncate(at); }

  FilePathSpan getRoot() const;
  FilePathSpan getDirectoryName() const;

  bool cdUp();

  FilePathSpan getFileName() const;
  FilePathSpan getFileNameWithoutExtension() const;

  void stripTrailingSeparators();

  int indexOfSeparator() const;
  int indexOfSeparator(int begin) const;
  int lastIndexOfSeparator() const;
  int lastIndexOfSeparator(int end) const;

  int indexOfDriveLetter() const;

  bool hasExtension() const;
  String getExtension() const;
  bool matchesExtension(StringSpan extension) const;
  void removeExtension();

  bool IsAbsolute() const { return getRootLength() > 0; }
  bool IsRelative() const { return !IsAbsolute(); }

  FilePathEnumerator enumerate() const;

  int getRootLength() const;
  int getDirectoryNameLength() const;
  int indexOfExtension() const;
  int countTrailingSeparators() const;

  friend bool operator<=(const FilePathSpan& l, const FilePathSpan& r) { return l.compareTo(r) <= 0; }
  friend bool operator>=(const FilePathSpan& l, const FilePathSpan& r) { return l.compareTo(r) >= 0; }
  friend bool operator< (const FilePathSpan& l, const FilePathSpan& r) { return l.compareTo(r) <  0; }
  friend bool operator> (const FilePathSpan& l, const FilePathSpan& r) { return l.compareTo(r) >  0; }

  friend bool operator==(const FilePathSpan& l, const FilePathSpan& r) { return l.equalsTo(r); }
  friend bool operator!=(const FilePathSpan& l, const FilePathSpan& r) { return !operator==(l, r); }
  friend HashCode partialHash(const FilePathSpan& x) { return x.hashImpl(); }
  friend int compare(const FilePathSpan& l, const FilePathSpan& r) { return l.compareTo(r); }
  friend TextWriter& operator<<(TextWriter& out, const FilePathSpan& x) {
    x.formatImpl(out);
    return out;
  }
  friend void format(TextWriter& out, const FilePathSpan& x, const StringSpan& opts) {
    x.formatImpl(out);
  }

 private:
  bool equalsTo(const FilePathSpan& other) const;
  int compareTo(const FilePathSpan& other) const;
  HashCode hashImpl() const;
  void formatImpl(TextWriter& out) const;

  Span<CharType> chars_;
};

inline FilePathSpan makeFilePathSpanFromNullTerminated(const FilePathChar* cstr) {
  return FilePathSpan(makeSpanFromNullTerminated(cstr));
}

class BASE_EXPORT FilePathEnumerator {
 public:
  explicit FilePathEnumerator(FilePathSpan path);

  FilePathSpan getCurrent() const { return path_.getSlice(now_pos_, now_len_); }

  FilePathEnumerator* next();

  class Iterator {
   public:
    explicit Iterator(FilePathEnumerator* e) : enumerator_(e) {}

    Iterator& operator++() { enumerator_ = enumerator_->next(); return *this; }
    FilePathSpan operator*() const { return enumerator_->getCurrent(); }

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

inline FilePathEnumerator FilePathSpan::enumerate() const {
  return FilePathEnumerator(*this);
}

} // namespace stp

#endif // STP_BASE_FS_FILEPATHSPAN_H_
