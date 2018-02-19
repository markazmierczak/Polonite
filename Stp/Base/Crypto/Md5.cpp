// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 */

#include "Base/Crypto/Md5.h"

#include "Base/Containers/Array.h"
#include "Base/Error/BasicExceptions.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Type/Formattable.h"

namespace stp {

static constexpr int NibbleCount = Md5Digest::Length * 2;

bool tryParse(StringSpan input, Md5Digest& out_digest) noexcept {
  if (input.size() != NibbleCount)
    return false;

  byte_t* outd = &out_digest[0];
  for (int i = 0; i < Md5Digest::Length; ++i) {
    int msb = tryParseHexDigit(input[i * 2 + 0]);
    int lsb = tryParseHexDigit(input[i * 2 + 1]);
    if (lsb < 0 || msb < 0)
      return false;
    outd[i] = static_cast<byte_t>((msb << 4) | lsb);
  }
  return true;
}

static void format(TextWriter& out, const Md5Digest& digest, bool uppercase) {
  Array<char, NibbleCount> text;
  for (int i = 0; i < Md5Digest::Length; ++i) {
    text[i * 2 + 0] = nibbleToHexDigit((digest[i] >> 4) & 0xF, uppercase);
    text[i * 2 + 1] = nibbleToHexDigit((digest[i] >> 0) & 0xF, uppercase);
  }
  out << text;
}

TextWriter& operator<<(TextWriter& out, const Md5Digest& digest) {
  format(out, digest, false);
  return out;
}

void format(TextWriter& out, const Md5Digest& digest, const StringSpan& opts) {
  bool uppercase = false;
  for (int i = 0; i < opts.size(); ++i) {
    char c = opts[i];
    switch (c) {
      case 'x':
      case 'X':
        uppercase = isUpperAscii(c);
        break;

      default:
        throw FormatException("Md5Digest");
    }
  }
  format(out, digest, uppercase);
}

// The four core functions - F1 is optimized somewhat.

// #define F1(x, y, z) (x & y | ~x & z)
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

// This is the central step in the MD5 algorithm.
#define MD5STEP(f, w, x, y, z, data, s) \
  (w += f(x, y, z) + data, w = w << s | w >> (32 - s), w += x)

/**
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.
 */
void Md5Hasher::transform() noexcept {
  uint32_t a, b, c, d;

  a = buf_[0];
  b = buf_[1];
  c = buf_[2];
  d = buf_[3];

  uint32_t* in = inw_;

  MD5STEP(F1, a, b, c, d, in[0] + 0xD76AA478, 7);
  MD5STEP(F1, d, a, b, c, in[1] + 0xE8C7B756, 12);
  MD5STEP(F1, c, d, a, b, in[2] + 0x242070DB, 17);
  MD5STEP(F1, b, c, d, a, in[3] + 0xC1BDCEEE, 22);
  MD5STEP(F1, a, b, c, d, in[4] + 0xF57C0FAF, 7);
  MD5STEP(F1, d, a, b, c, in[5] + 0x4787C62A, 12);
  MD5STEP(F1, c, d, a, b, in[6] + 0xA8304613, 17);
  MD5STEP(F1, b, c, d, a, in[7] + 0xFD469501, 22);
  MD5STEP(F1, a, b, c, d, in[8] + 0x698098D8, 7);
  MD5STEP(F1, d, a, b, c, in[9] + 0x8B44F7AF, 12);
  MD5STEP(F1, c, d, a, b, in[10] + 0xFFFF5BB1, 17);
  MD5STEP(F1, b, c, d, a, in[11] + 0x895CD7BE, 22);
  MD5STEP(F1, a, b, c, d, in[12] + 0x6B901122, 7);
  MD5STEP(F1, d, a, b, c, in[13] + 0xFD987193, 12);
  MD5STEP(F1, c, d, a, b, in[14] + 0xA679438E, 17);
  MD5STEP(F1, b, c, d, a, in[15] + 0x49B40821, 22);

  MD5STEP(F2, a, b, c, d, in[1] + 0xF61E2562, 5);
  MD5STEP(F2, d, a, b, c, in[6] + 0xC040B340, 9);
  MD5STEP(F2, c, d, a, b, in[11] + 0x265E5A51, 14);
  MD5STEP(F2, b, c, d, a, in[0] + 0xE9B6C7AA, 20);
  MD5STEP(F2, a, b, c, d, in[5] + 0xD62F105D, 5);
  MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
  MD5STEP(F2, c, d, a, b, in[15] + 0xD8A1E681, 14);
  MD5STEP(F2, b, c, d, a, in[4] + 0xE7D3FBC8, 20);
  MD5STEP(F2, a, b, c, d, in[9] + 0x21E1CDE6, 5);
  MD5STEP(F2, d, a, b, c, in[14] + 0xC33707D6, 9);
  MD5STEP(F2, c, d, a, b, in[3] + 0xF4D50D87, 14);
  MD5STEP(F2, b, c, d, a, in[8] + 0x455A14ED, 20);
  MD5STEP(F2, a, b, c, d, in[13] + 0xA9E3E905, 5);
  MD5STEP(F2, d, a, b, c, in[2] + 0xFCEFA3F8, 9);
  MD5STEP(F2, c, d, a, b, in[7] + 0x676F02D9, 14);
  MD5STEP(F2, b, c, d, a, in[12] + 0x8D2A4C8A, 20);

  MD5STEP(F3, a, b, c, d, in[5] + 0xFFFA3942, 4);
  MD5STEP(F3, d, a, b, c, in[8] + 0x8771F681, 11);
  MD5STEP(F3, c, d, a, b, in[11] + 0x6D9D6122, 16);
  MD5STEP(F3, b, c, d, a, in[14] + 0xFDE5380C, 23);
  MD5STEP(F3, a, b, c, d, in[1] + 0xA4BEEA44, 4);
  MD5STEP(F3, d, a, b, c, in[4] + 0x4BDECFA9, 11);
  MD5STEP(F3, c, d, a, b, in[7] + 0xF6BB4B60, 16);
  MD5STEP(F3, b, c, d, a, in[10] + 0xBEBFBC70, 23);
  MD5STEP(F3, a, b, c, d, in[13] + 0x289B7EC6, 4);
  MD5STEP(F3, d, a, b, c, in[0] + 0xEAA127FA, 11);
  MD5STEP(F3, c, d, a, b, in[3] + 0xD4EF3085, 16);
  MD5STEP(F3, b, c, d, a, in[6] + 0x04881D05, 23);
  MD5STEP(F3, a, b, c, d, in[9] + 0xD9D4D039, 4);
  MD5STEP(F3, d, a, b, c, in[12] + 0xE6DB99E5, 11);
  MD5STEP(F3, c, d, a, b, in[15] + 0x1FA27CF8, 16);
  MD5STEP(F3, b, c, d, a, in[2] + 0xC4AC5665, 23);

  MD5STEP(F4, a, b, c, d, in[0] + 0xF4292244, 6);
  MD5STEP(F4, d, a, b, c, in[7] + 0x432AFF97, 10);
  MD5STEP(F4, c, d, a, b, in[14] + 0xAB9423A7, 15);
  MD5STEP(F4, b, c, d, a, in[5] + 0xFC93A039, 21);
  MD5STEP(F4, a, b, c, d, in[12] + 0x655B59C3, 6);
  MD5STEP(F4, d, a, b, c, in[3] + 0x8F0CCC92, 10);
  MD5STEP(F4, c, d, a, b, in[10] + 0xFFEFF47D, 15);
  MD5STEP(F4, b, c, d, a, in[1] + 0x85845DD1, 21);
  MD5STEP(F4, a, b, c, d, in[8] + 0x6FA87E4F, 6);
  MD5STEP(F4, d, a, b, c, in[15] + 0xFE2CE6E0, 10);
  MD5STEP(F4, c, d, a, b, in[6] + 0xA3014314, 15);
  MD5STEP(F4, b, c, d, a, in[13] + 0x4E0811A1, 21);
  MD5STEP(F4, a, b, c, d, in[4] + 0xF7537E82, 6);
  MD5STEP(F4, d, a, b, c, in[11] + 0xBD3AF235, 10);
  MD5STEP(F4, c, d, a, b, in[2] + 0x2AD7D2BB, 15);
  MD5STEP(F4, b, c, d, a, in[9] + 0xEB86D391, 21);

  buf_[0] += a;
  buf_[1] += b;
  buf_[2] += c;
  buf_[3] += d;
}

/**
 * Start MD5 accumulation.
 * Set bit count to 0 and buffer to mysterious initialization constants.
 */
void Md5Hasher::reset() noexcept {
  buf_[0] = 0x67452301;
  buf_[1] = 0xEFCDAB89;
  buf_[2] = 0x98BADCFE;
  buf_[3] = 0x10325476;
  bits_[0] = 0;
  bits_[1] = 0;
}

/**
 * Update context to reflect the concatenation of another buffer full of bytes.
 */
void Md5Hasher::update(BufferSpan input) noexcept {
  // Update bitcount

  auto* d = static_cast<const byte_t*>(input.data());
  uint32_t size = input.size();

  uint32_t t = bits_[0];
  if ((bits_[0] = t + (size << 3)) < t)
    bits_[1]++; // Carry from low to high
  bits_[1] += size >> 29;

  t = (t >> 3) & 0x3F;

  // Handle any leading odd-sized chunks.

  if (t) {
    uint8_t* p = in_ + t;

    t = 64 - t;
    if (size < t) {
      memcpy(p, d, size);
      return;
    }
    memcpy(p, d, t);
    transform();
    d += t;
    size -= t;
  }

  // Process data in 64-byte chunks.

  while (size >= 64) {
    memcpy(in_, d, 64);
    transform();
    d += 64;
    size -= 64;
  }

  // Handle any remaining bytes of data.

  memcpy(in_, d, size);
}

/**
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void Md5Hasher::finish(Md5Digest& digest) noexcept {
  // Compute number of bytes mod 64.
  unsigned count = (bits_[0] >> 3) & 0x3F;

  // Set the first char of padding to 0x80.
  // This is safe since there is always at least one byte free.
  uint8_t* p = in_ + count;
  *p++ = 0x80;

  // Bytes of padding needed to make 64 bytes.
  count = 64 - 1 - count;

  // Pad out to 56 mod 64.
  if (count < 8) {
    // Two lots of padding:  Pad the first block to 64 bytes.
    memset(p, 0, count);
    transform();

    // Now fill the next block with 56 bytes.
    memset(in_, 0, 56);
  } else {
    // Pad block to 56 bytes.
    memset(p, 0, count - 8);
  }

  // Append length in bits and transform.
  copyObjectsNonOverlapping(inw_ + 14, bits_, 2);

  transform();
  memcpy(&digest[0], buf_, 16);
}

Md5Digest computeMd5Digest(BufferSpan input) noexcept {
  Md5Digest digest(Md5Digest::NoInit);
  Md5Hasher hasher;
  hasher.update(input);
  hasher.finish(digest);
  return digest;
}

} // namespace stp
