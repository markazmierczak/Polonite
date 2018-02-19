// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/Fixed.h"

#include "Base/Error/BasicExceptions.h"
#include "Base/Math/Bits.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/FormatMany.h"

namespace stp {
namespace detail {

int32_t SqrtFixed(int32_t x, int count) {
  ASSERT(x >= 0 && count > 0 && count <= 30);
  ASSERT(!(count & 1)); // Implemented only for even fractions.
  count = 15 + (count >> 1);

  uint32_t root = 0;
  uint32_t rem_hi = 0;
  uint32_t rem_lo = x;

  do {
    root <<= 1;

    rem_hi = (rem_hi << 2) | (rem_lo >> 30);
    rem_lo <<= 2;

    uint32_t test_div = (root << 1) + 1;
    if (rem_hi >= test_div) {
      rem_hi -= test_div;
      root++;
    }
  } while (--count >= 0);

  return root;
}

int64_t LSqrtFixed(int64_t x, int count) {
  ASSERT(x >= 0 && count > 0 && count <= 62);
  ASSERT(!(count & 1)); // Implemented only for even fractions.
  count = 31 + (count >> 1);

  uint64_t root = 0;
  uint64_t rem_hi = 0;
  uint64_t rem_lo = x;

  do {
    root <<= 1;

    rem_hi = (rem_hi << 2) | (rem_lo >> 62);
    rem_lo <<= 2;

    uint64_t test_div = (root << 1) + 1;
    if (rem_hi >= test_div) {
      rem_hi -= test_div;
      root++;
    }
  } while (--count >= 0);

  return root;
}

static const uint32_t RSqrt16Lookup[96] = {
  0xFA0BDEFA, 0xEE6AF6EE, 0xE5EFFAE5, 0xDAF27AD9,
  0xD2EFF6D0, 0xC890AEC4, 0xC10366BB, 0xB9A71AB2,
  0xB4DA2EAC, 0xADCE7EA3, 0xA6F2B29A, 0xA279A694,
  0x9BEB568B, 0x97A5C685, 0x9163067C, 0x8D4FD276,
  0x89501E70, 0x8563DA6A, 0x818AC664, 0x7DC4FE5E,
  0x7A122258, 0x7671BE52, 0x72E44A4C, 0x6F68FA46,
  0x6DB22A43, 0x6A52623D, 0x67041A37, 0x65639634,
  0x622FFE2E, 0x609CBA2B, 0x5D837E25, 0x5BFCFE22,
  0x58FD461C, 0x57838619, 0x560E1216, 0x53300A10,
  0x51C72E0D, 0x50621A0A, 0x4DA48204, 0x4C4C2E01,
  0x4AF789FE, 0x49A689FB, 0x485A11F8, 0x4710F9F5,
  0x45CC2DF2, 0x448B4DEF, 0x421505E9, 0x40DF5DE6,
  0x3FADC5E3, 0x3E7FE1E0, 0x3D55C9DD, 0x3D55D9DD,
  0x3C2F41DA, 0x39EDD9D4, 0x39EDC1D4, 0x38D281D1,
  0x37BAE1CE, 0x36A6C1CB, 0x3595D5C8, 0x3488F1C5,
  0x3488FDC5, 0x337FBDC2, 0x3279DDBF, 0x317749BC,
  0x307831B9, 0x307879B9, 0x2F7D01B6, 0x2E84DDB3,
  0x2D9005B0, 0x2D9015B0, 0x2C9EC1AD, 0x2BB0A1AA,
  0x2BB0F5AA, 0x2AC615A7, 0x29DED1A4, 0x29DEC9A4,
  0x28FABDA1, 0x2819E99E, 0x2819ED9E, 0x273C3D9B,
  0x273C359B, 0x2661DD98, 0x258AD195, 0x258AF195,
  0x24B71192, 0x24B6B192, 0x23E6058F, 0x2318118C,
  0x2318718C, 0x224DA189, 0x224DD989, 0x21860D86,
  0x21862586, 0x20C19183, 0x20C1B183, 0x20001580
};

int32_t RSqrtFixed16(int32_t xs) {
  ASSERT(xs > 0);
  uint32_t x = static_cast<uint32_t>(xs);

  uint32_t scale = countLeadingZeroBits(x);
  scale &= ~1;

  x <<= scale;

  auto UMulHi = [](uint32_t a, uint32_t b) {
    return static_cast<uint32_t>((static_cast<uint64_t>(a) * b) >> 32);
  };

  uint32_t t = RSqrt16Lookup[(x >> 25) - 32];

  uint32_t r = (t << 22) - UMulHi(t, x);
  uint32_t s = UMulHi(r, x);
  s = 0x30000000 - UMulHi(r, s);
  r = UMulHi(r, s);
  r = ((r >> (18 - (scale >> 1))) + 1) >> 1;
  return static_cast<int32_t>(r);
}


void FormatFixedPoint(TextWriter& out, int32_t value, int point) {
  out << (static_cast<double>(value) / (1 << point));
}

void FormatFixedPoint(TextWriter& out, const StringSpan& opts, int32_t value, int point) {
  if (opts.size() == 1 && toUpperAscii(opts[0]) == 'X') {
    char int_options[3] = { opts[0], '8', '\0' };
    format(out, static_cast<uint32_t>(value), StringSpan(int_options));
  } else {
    if (!opts.isEmpty())
      throw FormatException("Fixed");
    FormatFixedPoint(out, value, point);
  }
}

} // namespace detail
} // namespace stp
