// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/Comparable.h"

namespace stp {
namespace detail {

#if SIZEOF_WCHAR_T == 2
int CompareWcharArray(const wchar_t* lhs, const wchar_t* rhs, int size) {
  ASSERT(size >= 0);
  return size ? ::wmemcmp(lhs, rhs, ToUnsigned(size)) : 0;
}
#endif

} // namespace detail
} // namespace stp
