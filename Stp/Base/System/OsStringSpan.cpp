// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/OsStringSpan.h"

#include "Base/Type/Comparable.h"

namespace stp {

int OsStringSpan::indexOf(char c) const noexcept {
  for (int i = 0; i < length_; ++i) {
    if (data_[i] == c)
      return i;
  }
  return -1;
}

int OsStringSpan::lastIndexOf(char c) const noexcept {
  for (int i = length_ - 1; i >= 0; --i) {
    if (data_[i] == c)
      return i;
  }
  return -1;
}

bool operator==(const OsStringSpan& lhs, const OsStringSpan& rhs) noexcept {
  if (lhs.length() != rhs.length())
    return false;
  return equalObjects(lhs.data(), rhs.data(), lhs.length());
}

int compare(const OsStringSpan& lhs, const OsStringSpan& rhs) noexcept {
  int common_length = min(lhs.length(), rhs.length());
  if (common_length) {
    int rv = equalObjects(lhs.data(), rhs.data(), common_length);
    if (rv)
      return rv;
  }
  return compare(lhs.length(), rhs.length());
}

} // namespace stp
