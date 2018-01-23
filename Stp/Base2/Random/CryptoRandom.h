// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_RANDOM_CRYPTORANDOM_H_
#define STP_BASE_RANDOM_CRYPTORANDOM_H_

#include "Base/Export.h"
#include "Base/Type/Basic.h"

namespace stp {

class BASE_EXPORT CryptoRandom {
  STATIC_ONLY(CryptoRandom);
 public:
  // Fills |output_length| bytes of |output| with random data.
  static void NextBytes(byte_t* output, int output_length);

  static int Next();
  static int Next(int min, int max);

  // Returns a random number in range [0, UINT32_MAX]. Thread-safe.
  static uint32_t NextUInt32();

  // Returns a random number in range [0, UINT64_MAX]. Thread-safe.
  static uint64_t NextUInt64();

  // Returns a random number in range [0, range).  Thread-safe.
  //
  // Note that this can be used as an adapter for RandomShuffle():
  // Given a pre-populated |Vector<int> myvector|, shuffle it as
  //   RandomShuffle(myvector.begin(), myvector.end(), CryptoRandom::NextUInt64);
  static uint64_t NextUInt64(uint64_t range);

  // Returns a random floating-point number in range [0, 1). Thread-safe.
  static float NextFloat();
  static double NextDouble();
};

} // namespace stp

#endif // STP_BASE_RANDOM_CRYPTORANDOM_H_
