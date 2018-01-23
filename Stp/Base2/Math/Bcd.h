// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_BCD_H_
#define STP_BASE_MATH_BCD_H_

#include "Base/Debug/Assert.h"
#include "Base/Type/Basic.h"

namespace stp {

// Pair of BCD digits.
struct BcdPair {
  // Returns BCD representation of |byte| value.
  static constexpr BcdPair Pack(unsigned byte) {
    ASSERT(byte < 100);
    return { static_cast<uint8_t>(((byte / 10) << 4) | (byte % 10)) };
  }

  // Converts BCD representation to byte value.
  constexpr uint8_t Unpack() const {
    return (bits >> 4) * 10 + (bits & 0xF);
  }

  constexpr uint8_t GetLowDigit() const { return bits & 0xF; }
  constexpr uint8_t GetHighDigit() const { return bits >> 4; }

  uint8_t bits;
};

} // namespace stp

#endif // STP_BASE_MATH_BCD_H_
