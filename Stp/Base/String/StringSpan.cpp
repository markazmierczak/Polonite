// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/String/StringSpan.h"

#include "Base/Type/Comparable.h"

namespace stp {

int StringSpan::indexOf(char c) const noexcept {
  for (int i = 0; i < length_; ++i) {
    if (data_[i] == c)
      return i;
  }
  return -1;
}

int StringSpan::lastIndexOf(char c) const noexcept {
  for (int i = length_ - 1; i >= 0; --i) {
    if (data_[i] == c)
      return i;
  }
  return -1;
}

int compare(const StringSpan& lhs, const StringSpan& rhs) noexcept {
  int common_length = min(lhs.length(), rhs.length());
  if (common_length) {
    int rv = ::memcmp(lhs.data(), rhs.data(), toUnsigned(common_length));
    if (rv)
      return rv;
  }
  return compare(lhs.length(), rhs.length());
}

} // namespace stp
