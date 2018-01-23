// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Random/CryptoRandom.h"

#include "Base/Random/RandomInternal.h"
#include "Base/Type/Limits.h"

namespace stp {

int CryptoRandom::Next() {
  int number;
  NextBytes(reinterpret_cast<byte_t*>(&number), sizeof(number));
  return number;
}

uint32_t CryptoRandom::NextUInt32() {
  uint32_t number;
  NextBytes(reinterpret_cast<byte_t*>(&number), sizeof(number));
  return number;
}

uint64_t CryptoRandom::NextUInt64() {
  uint64_t number;
  NextBytes(reinterpret_cast<byte_t*>(&number), sizeof(number));
  return number;
}

uint64_t CryptoRandom::NextUInt64(uint64_t range) {
  ASSERT(range > 0);
  // We must discard random results above this number, as they would
  // make the random generator non-uniform (consider e.g. if
  // MAX_UINT64 was 7 and |range| was 5, then a result of 1 would be twice
  // as likely as a result of 3 or 4).
  uint64_t max_acceptable_value = (Limits<uint64_t>::Max / range) * range - 1;

  uint64_t value;
  do {
    value = NextUInt64();
  } while (value > max_acceptable_value);

  return value % range;
}

int CryptoRandom::Next(int min, int max) {
  ASSERT(min <= max);

  uint64_t range = static_cast<uint64_t>(max) - min + 1;
  // |range| is at most UINT_MAX + 1, so the result of rand_uint64(range)
  // is at most UINT_MAX.  Hence it's safe to cast it from uint64_t to int64_t.
  int result = static_cast<int>(min + static_cast<int64_t>(NextUInt64(range)));
  ASSERT(min <= result && result <= max);
  return result;
}

double CryptoRandom::NextDouble() {
  return detail::RandomBitsToDouble(NextUInt64());
}

float CryptoRandom::NextFloat() {
  return detail::RandomBitsToFloat(NextUInt32());
}

} // namespace stp
