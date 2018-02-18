// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/Hashable.h"

#include "Base/Math/BitsShift.h"

#include <string.h>

namespace stp {

static_assert(sizeof(HashCode) == 4, "hashing assumes HashCode is 4 bits wide");

HashCode finalizeHash(HashCode in_code) {
  auto code = toUnderlying(in_code);
  code ^= code >> 16;
  code *= UINT32_C(0x85EBCA6B);
  code ^= code >> 13;
  code *= UINT32_C(0xC2B2AE35);
  code ^= code >> 16;
  return static_cast<HashCode>(code);
}

HashCode combineHash(HashCode in_seed, HashCode in_value) {
  constexpr uint32_t C1 = UINT32_C(0xCC9E2D51);
  constexpr uint32_t C2 = UINT32_C(0x1B873593);

  auto seed = toUnderlying(in_seed);
  auto value = toUnderlying(in_value);

  value *= C1;
  value = RotateBitsRight(value, 15);
  value *= C2;

  seed ^= value;
  seed = RotateBitsRight(seed, 13);
  seed = seed * 5 + UINT32_C(0xE6546B64);

  return static_cast<HashCode>(seed);
}

static HashCode partialHash0To8(const byte_t* data, int size) {
  ASSERT(0 <= size && size <= 8);

  union {
    uint64_t x;
    uint8_t bytes[8];
  };
  x = 0;
  memcpy(bytes, data, size);
  return size <= isizeof(HashCode) ? static_cast<HashCode>(x) : partialHash(x);
}

HashCode hashBuffer(const void* data, int size) noexcept {
  ASSERT(size >= 0);
  auto* bytes = static_cast<const byte_t*>(data);
  if (size <= 8)
    return partialHash0To8(bytes, size);

  constexpr int SizeOfHashCode = sizeof(HashCode);
  HashCode rv = HashCode::Zero;
  int i;

  if ((intptr_t)bytes & (SizeOfHashCode - 1)) {
    // Specialization for unaligned access.
    for (i = 0; i < (size & -SizeOfHashCode); i += SizeOfHashCode) {
      HashCode data;
      memcpy(&data, bytes + i, SizeOfHashCode);
      rv = i == 0 ? data : combineHash(rv, data);
    }
    bytes += i;
  } else {
    // Specialization for aligned access.
    auto* words = reinterpret_cast<const HashCode*>(bytes);
    for (i = 0; i < (size & -SizeOfHashCode); i += SizeOfHashCode) {
      HashCode data = *words++;
      rv = i == 0 ? data : combineHash(rv, data);
    }
    bytes = reinterpret_cast<const byte_t*>(words);
  }
  if (i < size) {
    HashCode code = HashCode::Zero;
    memcpy(&code, bytes, size - i);
    rv = i == 0 ? code : combineHash(rv, code);
  }
  return rv;
}

} // namespace stp
