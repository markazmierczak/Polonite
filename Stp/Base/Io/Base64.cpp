// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2005, 2006 Nick Galbreath nickg@modp.com
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Io/Base64.h"

#include "Base/Io/Base64Data.h"

namespace stp {

String Base64::Encode(BufferSpan input) {
  String out;
  int estimated = EstimateEncodedLength(input.size());
  char* dst = out.appendUninitialized(estimated);
  int dst_length = Encode(MutableStringSpan(dst, estimated), input);
  out.truncate(dst_length);
  return out;
}

int Base64::Encode(MutableStringSpan output, BufferSpan input) {
  char* p = output.data();

  int input_size = input.size();
  auto* input_bytes = static_cast<const byte_t*>(input.data());

  int i = 0;

  byte_t t1, t2, t3;
  if (input_size > 2) {
    for (; i < input_size - 2; i += 3) {
      t1 = input_bytes[i + 0];
      t2 = input_bytes[i + 1];
      t3 = input_bytes[i + 2];
      *p++ = EncodeTable0[t1];
      *p++ = EncodeTable1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
      *p++ = EncodeTable1[((t2 & 0x0F) << 2) | ((t3 >> 6) & 0x03)];
      *p++ = EncodeTable2[t3];
    }
  }

  ASSUME(input_size - i < 3);
  switch (input_size - i) {
    case 0:
      break;

    case 1:
      t1 = input_bytes[i];
      *p++ = EncodeTable0[t1];
      *p++ = EncodeTable1[(t1 & 0x03) << 4];
      *p++ = CharPad;
      *p++ = CharPad;
      break;

    case 2:
      t1 = input_bytes[i + 0];
      t2 = input_bytes[i + 1];
      *p++ = EncodeTable0[t1];
      *p++ = EncodeTable1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
      *p++ = EncodeTable2[(t2 & 0x0F) << 2];
      *p++ = CharPad;
      break;
  }

  *p = '\0';

  int written = p - output.data();
  ASSERT(written <= output.size());
  return written;
}

bool Base64::TryDecode(StringSpan input, Buffer& output) {
  output.clear();

  int max_output_size = EstimateDecodedSize(input.size());
  void* dst = output.appendUninitialized(max_output_size);

  // Does not null terminate result since result is binary data!
  int actual_output_size = TryDecode(input, MutableBufferSpan(dst, max_output_size));
  if (actual_output_size < 0)
    return false;

  output.truncate(actual_output_size);
  return true;
}

static constexpr uint32_t BadChar = 0x01FFFFFF;

static inline uint8_t* WriteDecodedFour(uint8_t* p, uint32_t x) {
  *p++ = (x >>  0) & 0xFF;
  *p++ = (x >>  8) & 0xFF;
  *p++ = (x >> 16) & 0xFF;
  return p;
}

int Base64::TryDecode(StringSpan input, MutableBufferSpan output) {
  static constexpr int DecodeError = -1;

  int len = input.size();
  const char* src = input.data();
  auto* dst = static_cast<byte_t*>(output.data());

  ASSERT(len >= 0);
  if (len == 0)
    return 0;

  // Input must be at least 4 chars and be multiple of 4 to account padding.
  if (len & 3)
    return DecodeError;

  // There can be at most 2 pad chars at the end.
  if (src[len - 1] == CharPad) {
    len--;
    if (src[len - 1] == CharPad)
      len--;
  }

  auto* p = dst;

  int leftover = len & 3;
  int chunks = len >> 2;

  uint32_t x;
  auto* y = reinterpret_cast<const byte_t*>(src);
  for (int i = 0; i < chunks; ++i, y += 4) {
    x = DecodeTable0[y[0]] |
        DecodeTable1[y[1]] |
        DecodeTable2[y[2]] |
        DecodeTable3[y[3]];

    if (x >= BadChar)
      return DecodeError;
    p = WriteDecodedFour(p, x);
  }

  switch (leftover) {
    case 0:
      return 3 * chunks;

    case 2:
      x = DecodeTable0[y[0]] |
          DecodeTable1[y[1]];

      *p = (x >>  0) & 0xFF;
      break;

    case 3:
      x = DecodeTable0[y[0]] |
          DecodeTable1[y[1]] |
          DecodeTable2[y[2]];

      *p++ = (x >> 0) & 0xFF;
      *p   = (x >> 8) & 0xFF;
      break;

    default:
      UNREACHABLE(x = BadChar);
  }

  ASSERT(p - dst <= output.size());

  if (x >= BadChar)
    return DecodeError;

  return 3 * chunks + (6 * leftover) / 8;
}

} // namespace stp
