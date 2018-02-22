// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/FilePathSpan.h"

#include "Base/Error/BasicExceptions.h"
#include "Base/Type/Hashable.h"

namespace stp {

#if HAVE_FILE_PATH_WITH_DRIVE_LETTER
static int equalCaseInsensitive(const wchar_t* lhs, const wchar_t* rhs, int size) {
  int lhs_letter_pos = findDriveLetter(lhs, size);
  int rhs_letter_pos = findDriveLetter(rhs, size);

  if (lhs_letter_pos < 0 || rhs_letter_pos < 0)
    return RawWString::compare(lhs, rhs, size);

  ASSERT(lhs_letter_pos == rhs_letter_pos);

  int lhs_letter = toUpperAscii(lhs[lhs_letter_pos]);
  int rhs_letter = toUpperAscii(rhs[rhs_letter_pos]);
  if (lhs_letter != rhs_letter)
    return lhs_letter - rhs_letter;

  return RawWString::compare(
      lhs + lhs_letter_pos + 1,
      rhs + rhs_letter_pos + 1,
      size - (lhs_letter_pos + 1));
}
#endif

bool FilePathSpan::equalsTo(const FilePathSpan& other) const {
  if (size() != other.size())
    return false;
  #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
  return equalCaseInsensitive(data_, other.data(), size_) == 0;
  #else
  return chars_ == other.chars_;
  #endif
}

int FilePathSpan::compareTo(const FilePathSpan& other) const {
  // TODO not implemented
  throw NotImplementedException();
}

HashCode FilePathSpan::hashImpl() const {
  return partialHash(chars_);
}

} // namespace stp
