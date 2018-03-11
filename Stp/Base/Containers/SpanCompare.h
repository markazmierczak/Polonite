// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_SPANCOMPARE_H_
#define STP_BASE_CONTAINERS_SPANCOMPARE_H_

#include "Base/Containers/Span.h"
#include "Base/Type/Comparable.h"

namespace stp {

template<typename T, typename U = T>
inline int compareSpans(Span<T> lhs, Span<U> rhs) {
  int common = min(lhs.size(), rhs.size());
  for (int i = 0; i < common; ++i) {
    int rv = compare(lhs[i], rhs[i]);
    if (rv)
      return rv;
  }
  return compare(lhs.size(), rhs.size());
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_SPANCOMPARE_H_
