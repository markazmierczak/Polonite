// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Crypto/Sha1.h"

#include "Base/Containers/Buffer.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(Sha1Test, OneBlockMessage) {
  // Example A.1 from FIPS 180-2: one-block message.
  auto input = BufferSpan("abc");

  Sha1Digest expected({
    0xA9, 0x99, 0x3E, 0x36,
    0x47, 0x06, 0x81, 0x6A,
    0xBA, 0x3E, 0x25, 0x71,
    0x78, 0x50, 0xC2, 0x6C,
    0x9C, 0xD0, 0xD8, 0x9D
  });

  auto output = ComputeSha1Digest(input);
  for (int i = 0; i < Sha1Digest::Length; i++)
    EXPECT_EQ(expected, output);
}

TEST(Sha1Test, MultiBlockMessage) {
  // Example A.2 from FIPS 180-2: multi-block message.
  auto input = BufferSpan("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");

  Sha1Digest expected({
    0x84, 0x98, 0x3E, 0x44,
    0x1C, 0x3B, 0xD2, 0x6E,
    0xBA, 0xAE, 0x4A, 0xA1,
    0xF9, 0x51, 0x29, 0xE5,
    0xE5, 0x46, 0x70, 0xF1
  });

  auto output = ComputeSha1Digest(input);
  for (int i = 0; i < Sha1Digest::Length; i++)
    EXPECT_EQ(expected, output);
}

TEST(Sha1Test, LongMessage) {
  // Example A.3 from FIPS 180-2: long message.

  Buffer input;
  void* ptr = input.AppendUninitialized(1000000);
  memset(ptr, 'a', input.size());

  Sha1Digest expected({
    0x34, 0xAA, 0x97, 0x3C,
    0xD4, 0xC4, 0xDA, 0xA4,
    0xF6, 0x1E, 0xEB, 0x2B,
    0xDB, 0xAD, 0x27, 0x31,
    0x65, 0x34, 0x01, 0x6F
  });

  auto output = ComputeSha1Digest(input);
  for (int i = 0; i < Sha1Digest::Length; i++)
    EXPECT_EQ(expected, output);
}

} // namespace stp
