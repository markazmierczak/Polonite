// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/HashMap.h"

#include "Base/Io/TextWriter.h"
#include "Base/Math/PowerOfTwo.h"
#include "Base/Math/Prime.h"

namespace stp {
namespace detail {

int HashMapBase::OptimalBucketCount(unsigned min, bool binary_size) {
  if (min == 0)
    return 0;

  int count;
  if (binary_size) {
    count = RoundUpToPowerOfTwo(min);
  } else {
    count = NextPrimeNumber(min - 1);
  }
  return count;
}

} // namespace detail
} // namespace stp
