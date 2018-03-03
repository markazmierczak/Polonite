// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Crypto/Crc32.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(Crc32Test, Basic) {
  struct {
    BufferSpan bytes;
    uint32_t output;
  } cases[] = {
    {BufferSpan(""), UINT32_C(0x00000000)},
    {BufferSpan("a"), UINT32_C(0xE8B7BE43)},
    {BufferSpan("abc"), UINT32_C(0x352441C2)},
    {BufferSpan("message digest"), UINT32_C(0x20159D7F)},
    {BufferSpan("abcdefghijklmnopqrstuvwxyz"), UINT32_C(0x4C2750BD)},
    {BufferSpan("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"),
     UINT32_C(0x1FC2E6D2)},
    {BufferSpan("12345678901234567890123456789012345678901234567890123456789012345678901234567890"),
     UINT32_C(0x7CA94A72)},
    {BufferSpan("123456789"), UINT32_C(0xCBF43926)},
  };

  for (const auto& item : cases) {
    EXPECT_EQ(item.output, toUnderlying(computeCrc32(item.bytes)));
  }
}

} // namespace stp
