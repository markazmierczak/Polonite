// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Crypto/Crc32.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(Crc32Test, Basic) {
  struct {
    StringSpan bytes;
    uint32_t output;
  } cases[] = {
    {"", UINT32_C(0x00000000)},
    {"a", UINT32_C(0xE8B7BE43)},
    {"abc", UINT32_C(0x352441C2)},
    {"message digest", UINT32_C(0x20159D7F)},
    {"abcdefghijklmnopqrstuvwxyz", UINT32_C(0x4C2750BD)},
    {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
     UINT32_C(0x1FC2E6D2)},
    {"12345678901234567890123456789012345678901234567890123456789012345678901234567890",
     UINT32_C(0x7CA94A72)},
    {"123456789", UINT32_C(0xCBF43926)},
  };

  for (const auto& item : cases) {
    EXPECT_EQ(item.output, ToUnderlying(ComputeCrc32(BufferSpan(item.bytes))));
  }
}

} // namespace stp
