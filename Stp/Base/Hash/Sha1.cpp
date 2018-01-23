// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Hash/Sha1.h"

#include "Base/Containers/Array.h"
#include "Base/Error/BasicExceptions.h"
#include "Base/Io/TextWriter.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Util/SwapBytes.h"

namespace stp {

static constexpr int NibbleCount = Sha1Digest::Length * 2;

bool TryParse(StringSpan input, Sha1Digest& out_digest) {
  if (input.size() != NibbleCount)
    return false;

  byte_t* outd = &out_digest[0];
  for (int i = 0; i < Sha1Digest::Length; ++i) {
    int msb = TryParseHexDigit(input[i * 2 + 0]);
    int lsb = TryParseHexDigit(input[i * 2 + 1]);
    if (lsb < 0 || msb < 0)
      return false;
    outd[i] = static_cast<byte_t>((msb << 4) | lsb);
  }
  return true;
}

static void Format(TextWriter& out, const Sha1Digest& digest, bool uppercase) {
  Array<char, NibbleCount> text;
  for (int i = 0; i < Sha1Digest::Length; ++i) {
    text[i * 2 + 0] = NibbleToHexDigit((digest[i] >> 4) & 0xF, uppercase);
    text[i * 2 + 1] = NibbleToHexDigit((digest[i] >> 0) & 0xF, uppercase);
  }
  out.WriteAscii(text);
}

void Format(TextWriter& out, const Sha1Digest& digest) {
  Format(out, digest, false);
}

void Format(TextWriter& out, const Sha1Digest& digest, const StringSpan& opts) {
  bool uppercase = false;
  for (int i = 0; i < opts.size(); ++i) {
    char c = opts[i];
    switch (c) {
      case 'x':
      case 'X':
        uppercase = IsUpperAscii(c);
        break;

      default:
        throw FormatException("Sha1Digest");
    }
  }
  Format(out, digest, uppercase);
}

void Sha1Hasher::Reset() {
  a_ = 0;
  b_ = 0;
  c_ = 0;
  d_ = 0;
  e_ = 0;
  cursor_ = 0;
  l_ = 0;
  h_[0] = 0x67452301;
  h_[1] = 0xEFCDAB89;
  h_[2] = 0x98BADCFE;
  h_[3] = 0x10325476;
  h_[4] = 0xC3D2E1F0;
}

void Sha1Hasher::Finish(Digest& out_digest) {
  Pad();
  Process();

  for (int t = 0; t < 5; ++t)
    h_[t] = SwapBytes(h_[t]);

  memcpy(&out_digest[0], h_, Sha1Digest::Length);
}

void Sha1Hasher::Update(BufferSpan buffer) {
  auto* bytes = static_cast<const byte_t*>(buffer.data());
  int nbytes = buffer.size();

  while (nbytes--) {
    m_[cursor_++] = *bytes++;
    if (cursor_ >= 64)
      Process();
    l_ += 8;
  }
}

void Sha1Hasher::Pad() {
  m_[cursor_++] = 0x80;

  if (cursor_ > 64-8) {
    // pad out to next block
    while (cursor_ < 64)
      m_[cursor_++] = 0;

    Process();
  }

  while (cursor_ < 64-8)
    m_[cursor_++] = 0;

  m_[cursor_++] = (l_ >> 56) & 0xFF;
  m_[cursor_++] = (l_ >> 48) & 0xFF;
  m_[cursor_++] = (l_ >> 40) & 0xFF;
  m_[cursor_++] = (l_ >> 32) & 0xFF;
  m_[cursor_++] = (l_ >> 24) & 0xFF;
  m_[cursor_++] = (l_ >> 16) & 0xFF;
  m_[cursor_++] = (l_ >> 8) & 0xFF;
  m_[cursor_++] = l_ & 0xFF;
}

static inline uint32_t f(uint32_t t, uint32_t b, uint32_t c, uint32_t d) {
  if (t < 20)
    return (b & c) | ((~b) & d);
  if (t < 40)
    return b ^ c ^ d;
  if (t < 60)
    return (b & c) | (b & d) | (c & d);
  return b ^ c ^ d;
}

static inline uint32_t S(uint32_t n, uint32_t x) {
  return (x << n) | (x >> (32-n));
}

static inline uint32_t K(uint32_t t) {
  if (t < 20)
    return 0x5A827999;
  if (t < 40)
    return 0x6ED9EBA1;
  if (t < 60)
    return 0x8F1BBCDC;

  return 0xCA62C1D6;
}

void Sha1Hasher::Process() {
  // Each a...e corresponds to a section in the FIPS 180-3 algorithm.

  // a.
  //
  // W and M are in a union, so no need to memcpy.
  // memcpy(W, M, sizeof(M));
  for (int t = 0; t < 16; ++t)
    w_[t] = SwapBytes(w_[t]);

  // b.
  for (int t = 16; t < 80; ++t)
    w_[t] = S(1, w_[t - 3] ^ w_[t - 8] ^ w_[t - 14] ^ w_[t - 16]);

  // c.
  a_ = h_[0];
  b_ = h_[1];
  c_ = h_[2];
  d_ = h_[3];
  e_ = h_[4];

  // d.
  for (int t = 0; t < 80; ++t) {
    uint32_t temp = S(5, a_) + f(t, b_, c_, d_) + e_ + w_[t] + K(t);
    e_ = d_;
    d_ = c_;
    c_ = S(30, b_);
    b_ = a_;
    a_ = temp;
  }

  // e.
  h_[0] += a_;
  h_[1] += b_;
  h_[2] += c_;
  h_[3] += d_;
  h_[4] += e_;

  cursor_ = 0;
}

Sha1Digest ComputeSha1Digest(BufferSpan input) {
  Sha1Digest digest(NoInit);
  Sha1Hasher hasher;
  hasher.Update(input);
  hasher.Finish(digest);
  return digest;
}

} // namespace stp
