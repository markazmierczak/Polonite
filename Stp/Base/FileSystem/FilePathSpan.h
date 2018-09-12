// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_FILEPATHSPAN_H_
#define STP_BASE_FS_FILEPATHSPAN_H_

#include "Base/System/OsStringSpan.h"
#include "Base/Type/Nullable.h"

namespace stp {

class FilePath;
class FilePathComponents;

#if OS(POSIX)
# define FILE_PATH_LITERAL(x) x
#elif OS(WIN)
# define FILE_PATH_LITERAL(x) L ## x
#endif

// Windows-style drive letter support and pathname separator characters can be
// enabled and disabled independently, to aid testing.  These #defines are
// here so that the same setting can be used in both the implementation and
// in the unit test.
#define HAVE_FILE_PATH_WITH_DRIVE_LETTER (OS(WIN))

#if OS(POSIX)
constexpr OsChar FilePathSeparator = '/';
constexpr OsChar FilePathAltSeparator = '/';
#elif OS(WIN)
constexpr OsChar FilePathSeparator = L'\\';
constexpr OsChar FilePathAltSeparator = L'/';
#endif

constexpr bool isFilePathSeparator(OsChar c) {
  return c == FilePathSeparator || c == FilePathAltSeparator;
}

class FilePathSpan {
 public:
  constexpr FilePathSpan() = default;

  constexpr FilePathSpan(const OsChar* data, int size) noexcept : str_(data, size) {}
  constexpr explicit FilePathSpan(OsStringSpan str) noexcept : str_(str) {}

  ALWAYS_INLINE constexpr const OsChar* data() const noexcept { return str_.data(); }
  ALWAYS_INLINE constexpr int length() const noexcept { return str_.length(); }

  // Returns underlying characters in native encoding.
  // Be very careful on using this. See documentation beforehand.
  ALWAYS_INLINE const OsStringSpan& asOsStr() const noexcept { return str_; }

  constexpr bool isEmpty() const noexcept { return str_.isEmpty(); }

  FilePathSpan getRoot() const noexcept;
  FilePathSpan getDirectoryName() const noexcept;

  bool cdUp() noexcept;

  Nullable<OsStringSpan> fileName() const noexcept;
  Nullable<OsStringSpan> fileStem() const noexcept;

  void stripTrailingSeparators() noexcept;

  int indexOfSeparator() const noexcept;
  int indexOfSeparator(int begin) const noexcept;
  int lastIndexOfSeparator() const noexcept;
  int lastIndexOfSeparator(int end) const noexcept;

  int indexOfDriveLetter() const noexcept;

  Nullable<OsStringSpan> extension() const noexcept;
  bool matchesExtension(StringSpan extension) const noexcept;
  void removeExtension() noexcept;

  bool isAbsolute() const noexcept { return getRootLength() > 0; }
  bool IsRelative() const noexcept { return !isAbsolute(); }

  FilePathComponents components() const noexcept;

  int getRootLength() const noexcept;
  int getDirectoryNameLength() const noexcept;
  int indexOfExtension() const noexcept;
  int countTrailingSeparators() const noexcept;

  friend bool operator<=(const FilePathSpan& l, const FilePathSpan& r) noexcept { return l.compareTo(r) <= 0; }
  friend bool operator>=(const FilePathSpan& l, const FilePathSpan& r) noexcept { return l.compareTo(r) >= 0; }
  friend bool operator< (const FilePathSpan& l, const FilePathSpan& r) noexcept { return l.compareTo(r) <  0; }
  friend bool operator> (const FilePathSpan& l, const FilePathSpan& r) noexcept { return l.compareTo(r) >  0; }

  friend bool operator==(const FilePathSpan& l, const FilePathSpan& r) noexcept { return l.equalsTo(r); }
  friend bool operator!=(const FilePathSpan& l, const FilePathSpan& r) noexcept { return !operator==(l, r); }
  friend HashCode partialHash(const FilePathSpan& x) noexcept { return partialHash(x.str_); }
  friend int compare(const FilePathSpan& l, const FilePathSpan& r) noexcept { return l.compareTo(r); }
  friend TextWriter& operator<<(TextWriter& out, const FilePathSpan& x) {
    x.formatImpl(out);
    return out;
  }
  friend void format(TextWriter& out, const FilePathSpan& x, const StringSpan& opts) {
    x.formatImpl(out);
  }

 private:
  OsStringSpan str_;

  BASE_EXPORT bool equalsTo(const FilePathSpan& other) const noexcept;
  BASE_EXPORT int compareTo(const FilePathSpan& other) const noexcept;
  BASE_EXPORT void formatImpl(TextWriter& out) const;
};

class FilePathComponents {
 public:
  explicit FilePathComponents(FilePathSpan path) noexcept;

  FilePathSpan getCurrent() const { return path_.slice(now_pos_, now_len_); }

  FilePathComponents* next() noexcept;

  class Iterator {
   public:
    explicit Iterator(FilePathComponents* e) noexcept : enumerator_(e) {}

    Iterator& operator++() noexcept { enumerator_ = enumerator_->next(); return *this; }
    FilePathSpan operator*() const noexcept { return enumerator_->getCurrent(); }

    bool operator==(const Iterator& o) const noexcept { return enumerator_ == o.enumerator_; }
    bool operator!=(const Iterator& o) const noexcept { return enumerator_ != o.enumerator_; }

   private:
    FilePathComponents* enumerator_;
  };

 private:
  FilePathSpan path_;
  int now_pos_ = 0;
  int now_len_;
};

inline FilePathComponents FilePathSpan::components() const noexcept {
  return FilePathComponents(*this);
}

} // namespace stp

#endif // STP_BASE_FS_FILEPATHSPAN_H_
