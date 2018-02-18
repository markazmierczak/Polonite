// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/FilePathSpan.h"

#include "Base/Error/BasicExceptions.h"
#include "Base/Type/Hashable.h"

namespace stp {

#if HAVE_FILE_PATH_WITH_DRIVE_LETTER
static int EqualCaseInsensitive(const wchar_t* lhs, const wchar_t* rhs, int size) {
  int lhs_letter_pos = FindDriveLetter(lhs, size);
  int rhs_letter_pos = FindDriveLetter(rhs, size);

  if (lhs_letter_pos < 0 || rhs_letter_pos < 0)
    return RawWString::compare(lhs, rhs, size);

  ASSERT(lhs_letter_pos == rhs_letter_pos);

  int lhs_letter = ToUpperAscii(lhs[lhs_letter_pos]);
  int rhs_letter = ToUpperAscii(rhs[rhs_letter_pos]);
  if (lhs_letter != rhs_letter)
    return lhs_letter - rhs_letter;

  return RawWString::compare(
      lhs + lhs_letter_pos + 1,
      rhs + rhs_letter_pos + 1,
      size - (lhs_letter_pos + 1));
}
#endif

bool FilePathSpan::EqualsTo(const FilePathSpan& other) const {
  if (size() != other.size())
    return false;
  #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
  return EqualCaseInsensitive(data_, other.data(), size_) == 0;
  #else
  return chars_ == other.chars_;
  #endif
}

int FilePathSpan::CompareTo(const FilePathSpan& other) const {
  // TODO not implemented
  throw NotImplementedException();
}

HashCode FilePathSpan::HashImpl() const {
  return partialHash(chars_);
}

} // namespace stp
