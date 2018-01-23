// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_RANDOM_RANDOMSHUFFLE_H_
#define STP_BASE_RANDOM_RANDOMSHUFFLE_H_

#include "Base/Type/Variable.h"

namespace stp {

template<class RandomAccessIterator, class RandomNumberGenerator>
void RandomShuffle(
    RandomAccessIterator first, RandomAccessIterator last,
    RandomNumberGenerator&& generator) {
  for (int diff = last - first; diff > 0; ++first, --diff) {
    int i = generator(diff);
    if (i != 0)
      Swap(*first, *(first + i));
  }
}

} // namespace stp

#endif // STP_BASE_RANDOM_RANDOMSHUFFLE_H_
