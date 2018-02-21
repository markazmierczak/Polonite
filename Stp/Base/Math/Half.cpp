// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/Half.h"

#include "Base/Math/RawFloat.h"
#include "Base/Type/Formattable.h"

namespace stp {

// based on Fabien Giesen's float_to_half_fast3()
// see https://gist.github.com/rygorous/2156668
Half::Half(float x) {
  RawFloat f(x);
  uint32_t fbits = f.toBits();
  uint32_t sign = f.getSignBit();
  fbits ^= sign;

  // NOTE all the integer compares in this function can be safely
  // compiled into signed compares since all operands are below
  // 0x80000000. Important if you want fast straight SSE2 code
  // (since there's no unsigned PCMPGTD).

  uint16_t result;
  // Inf or NaN (all exponent bits set)
  if (fbits >= RawFloat::ExponentBitMask) {
    // NaN->qNaN and Inf->Inf
    result = (fbits > RawFloat::ExponentBitMask) ? 0x7E00u : 0x7C00;
    // (De)normalized number or zero
  } else {
    constexpr uint32_t RoundMask = ~0xFFFu;
    constexpr RawFloat Magic = RawFloat::fromBits(15u << 23);

    fbits &= RoundMask;

    auto tmp = RawFloat::fromBits(fbits);
    fbits = RawFloat(static_cast<float>(tmp) * static_cast<float>(Magic)).toBits();
    fbits -= RoundMask;

    constexpr RawFloat Infinity16 = RawFloat::fromBits(31u << 23);
    // Clamp to signed infinity if overflowed
    if (fbits > Infinity16.toBits())
      fbits = Infinity16.toBits();

    result = fbits >> 13; // Take the bits!
  }

  result |= sign >> 16;
  bits_ = result;
}

// based on Fabien Giesen's half_to_float_fast2()
// see https://fgiesen.wordpress.com/2012/03/28/half-to-float-done-quic/
Half::operator float() const {
  uint32_t obits;

  if (getExponentBits() == 0) {
    constexpr RawFloat Magic = RawFloat::fromBits(126u << 23);
    // Zero / Denormal
    auto dm = RawFloat::fromBits(Magic.toBits() + getMantissaBits());

    obits = RawFloat(static_cast<float>(dm) - static_cast<float>(Magic)).toBits();
  } else {
    // Set mantissa
    obits = getMantissaBits() << 13;
    // Set exponent
    if (getExponentBits() == 0x1F) {
      // Inf/NaN
      obits |= RawFloat::ExponentBitMask;
    } else {
      obits |= (127 - 15 + getExponentBits()) << 23;
    }
  }

  // Set sign
  obits |= static_cast<uint32_t>(getSignBit()) << 16;
  return static_cast<float>(RawFloat::fromBits(obits));
}

void Half::formatImpl(TextWriter& out, float x, const StringSpan& opts) {
  format(out, x, opts);
}

} // namespace stp
