// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/Span.h"

#include "Base/Type/Comparable.h"

namespace stp {

int compare(const StringSpan& lhs, const StringSpan& rhs) noexcept {
  int common_size = min(lhs.size(), rhs.size());
  if (common_size) {
    int rv = ::memcmp(lhs.data(), rhs.data(), toUnsigned(common_size));
    if (rv)
      return rv;
  }
  return compare(lhs.size(), rhs.size());
}

} // namespace stp
